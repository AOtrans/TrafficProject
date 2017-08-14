#include "cartracker.h"

extern "C"
{
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}


#define STREAM_FRAME_RATE 25 /* 25 images/s */

#define SCALE_FLAGS SWS_BICUBIC

// a wrapper around a single output AVStream
typedef struct OutputStream {
    AVStream *st;
    AVCodecContext *enc;

    /* pts of the next frame that will be generated */
    int64_t next_pts;
    int samples_count;

    AVFrame *frame;
    AVFrame *tmp_frame;

    float t, tincr, tincr2;

    struct SwsContext *sws_ctx;
    struct SwrContext *swr_ctx;
} OutputStream;

void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt)
{
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

//    printf("pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
//           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
//           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
//           av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
//           pkt->stream_index);
}

int write_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt)
{
    /* rescale output packet timestamp values from codec to stream timebase */
    av_packet_rescale_ts(pkt, *time_base, st->time_base);
    pkt->stream_index = st->index;

    /* Write the compressed frame to the media file. */
    log_packet(fmt_ctx, pkt);
    return av_interleaved_write_frame(fmt_ctx, pkt);
}

/* Add an output stream. */
void add_stream(OutputStream *ost, AVFormatContext *oc,
                       AVCodec **codec,
                       enum AVCodecID codec_id, int width, int height)
{
    AVCodecContext *c;
    int i;

    /* find the encoder */
    *codec = avcodec_find_encoder(codec_id);
    if (!(*codec)) {
        fprintf(stderr, "Could not find encoder for '%s'\n",
                avcodec_get_name(codec_id));
        exit(1);
    }

    ost->st = avformat_new_stream(oc, NULL);
    if (!ost->st) {
        fprintf(stderr, "Could not allocate stream\n");
        exit(1);
    }
    ost->st->id = oc->nb_streams-1;
    c = avcodec_alloc_context3(*codec);
    if (!c) {
        fprintf(stderr, "Could not alloc an encoding context\n");
        exit(1);
    }
    ost->enc = c;

    switch ((*codec)->type) {
    case AVMEDIA_TYPE_AUDIO:
        c->sample_fmt  = (*codec)->sample_fmts ?
            (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
        c->bit_rate    = 64000;
        c->sample_rate = 44100;
        if ((*codec)->supported_samplerates) {
            c->sample_rate = (*codec)->supported_samplerates[0];
            for (i = 0; (*codec)->supported_samplerates[i]; i++) {
                if ((*codec)->supported_samplerates[i] == 44100)
                    c->sample_rate = 44100;
            }
        }
        c->channels        = av_get_channel_layout_nb_channels(c->channel_layout);
        c->channel_layout = AV_CH_LAYOUT_STEREO;
        if ((*codec)->channel_layouts) {
            c->channel_layout = (*codec)->channel_layouts[0];
            for (i = 0; (*codec)->channel_layouts[i]; i++) {
                if ((*codec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
                    c->channel_layout = AV_CH_LAYOUT_STEREO;
            }
        }
        c->channels        = av_get_channel_layout_nb_channels(c->channel_layout);
        ost->st->time_base = (AVRational){ 1, c->sample_rate };
        break;

    case AVMEDIA_TYPE_VIDEO:
        c->codec_id = codec_id;

        c->bit_rate = 400000;
        /* Resolution must be a multiple of two. */
        c->width    = width;
        c->height   = height;
        /* timebase: This is the fundamental unit of time (in seconds) in terms
         * of which frame timestamps are represented. For fixed-fps content,
         * timebase should be 1/framerate and timestamp increments should be
         * identical to 1. */
        ost->st->time_base = (AVRational){ 1, STREAM_FRAME_RATE };
        c->time_base       = ost->st->time_base;

        c->gop_size      = 12; /* emit one intra frame every twelve frames at most */
        c->pix_fmt       = AV_PIX_FMT_YUV420P;
        if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
            /* just for testing, we also add B-frames */
            c->max_b_frames = 2;
        }
        if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
            /* Needed to avoid using macroblocks in which some coeffs overflow.
             * This does not happen with normal video, it just happens here as
             * the motion of the chroma plane does not match the luma plane. */
            c->mb_decision = 2;
        }
    break;

    default:
        break;
    }

    /* Some formats want stream headers to be separate. */
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
}

/**************************************************************/
/* video output */

AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
{
    AVFrame *picture;
    int ret;

    picture = av_frame_alloc();
    if (!picture)
        return NULL;

    picture->format = pix_fmt;
    picture->width  = width;
    picture->height = height;

    /* allocate the buffers for the frame data */
    ret = av_frame_get_buffer(picture, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate frame data.\n");
        exit(1);
    }

    return picture;
}

void open_video(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg)
{
    int ret;
    AVCodecContext *c = ost->enc;
    AVDictionary *opt = NULL;

    av_dict_copy(&opt, opt_arg, 0);

    /* open the codec */
    ret = avcodec_open2(c, codec, &opt);
    av_dict_free(&opt);
    if (ret < 0) {
        cout << "Could not open video codec: %s\n" << endl;
        exit(1);
    }

    /* allocate and init a re-usable frame */
    ost->frame = alloc_picture(c->pix_fmt, c->width, c->height);
    if (!ost->frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }

    /* If the output format is not YUV420P, then a temporary YUV420P
     * picture is needed too. It is then converted to the required
     * output format. */
    ost->tmp_frame = NULL;
    if (c->pix_fmt != AV_PIX_FMT_YUV420P) {
        ost->tmp_frame = alloc_picture(AV_PIX_FMT_YUV420P, c->width, c->height);
        if (!ost->tmp_frame) {
            fprintf(stderr, "Could not allocate temporary picture\n");
            exit(1);
        }
    }

    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(ost->st->codecpar, c);
    if (ret < 0) {
        fprintf(stderr, "Could not copy the stream parameters\n");
        exit(1);
    }
}

/* Prepare a dummy image. */
void fill_yuv_image(AVFrame *pict, cv::Mat src)
{
    cv::Mat yuv;
    cv::cvtColor(src, yuv, CV_BGR2YUV_I420);

    avpicture_fill((AVPicture*)pict, (uint8_t*)yuv.data, AV_PIX_FMT_YUV420P, pict->width, pict->height);
}

AVFrame *get_video_frame(OutputStream *ost, cv::Mat src)
{
    AVCodecContext *c = ost->enc;

//    /* check if we want to generate more frames */
//    if (av_compare_ts(ost->next_pts, c->time_base,
//                      STREAM_DURATION, (AVRational){ 1, 1 }) >= 0)
//        return NULL;

    /* when we pass a frame to the encoder, it may keep a reference to it
     * internally; make sure we do not overwrite it here */
    if (av_frame_make_writable(ost->frame) < 0)
        exit(1);

    if (c->pix_fmt != AV_PIX_FMT_YUV420P) {
        /* as we only generate a YUV420P picture, we must convert it
         * to the codec pixel format if needed */
        if (!ost->sws_ctx) {
            ost->sws_ctx = sws_getContext(c->width, c->height,
                                          AV_PIX_FMT_YUV420P,
                                          c->width, c->height,
                                          c->pix_fmt,
                                          SCALE_FLAGS, NULL, NULL, NULL);
            if (!ost->sws_ctx) {
                fprintf(stderr,
                        "Could not initialize the conversion context\n");
                exit(1);
            }
        }
        fill_yuv_image(ost->tmp_frame, src);
        //*(ost->tmp_frame) = *frame;
        sws_scale(ost->sws_ctx,
                  (const uint8_t * const *)ost->tmp_frame->data, ost->tmp_frame->linesize,
                  0, c->height, ost->frame->data, ost->frame->linesize);
    } else {
        fill_yuv_image(ost->frame, src);
        //*(ost->frame) = *frame;
    }

    ost->frame->pts = ost->next_pts++;

    return ost->frame;
}

/*
 * encode one video frame and send it to the muxer
 * return 1 when encoding is finished, 0 otherwise
 */
void write_video_frame(AVFormatContext *oc, OutputStream *ost, cv::Mat src)
{
    int ret;
    AVCodecContext *c;
    int got_packet = 0;
    AVPacket pkt = { 0 };
    AVFrame *frame;

    c = ost->enc;

    frame = get_video_frame(ost ,src);

    av_init_packet(&pkt);

    /* encode the image */
    ret = avcodec_encode_video2(c, &pkt, frame, &got_packet);
    if (ret < 0) {
        cout <<"Error encoding video frame: %s\n" <<endl;
        exit(1);
    }

    if (got_packet) {
        ret = write_frame(oc, &c->time_base, ost->st, &pkt);
    } else {
        ret = 0;
    }

    if (ret < 0) {
        cout << "Error while writing video frame: %s\n" << endl;
        exit(1);
    }
    else
    {
        cout << "______________________________write in_____________________________" <<endl;
    }
}

void close_stream(AVFormatContext *oc, OutputStream *ost)
{
    avcodec_free_context(&ost->enc);
    av_frame_free(&ost->frame);
    av_frame_free(&ost->tmp_frame);
    sws_freeContext(ost->sws_ctx);
    swr_free(&ost->swr_ctx);
}


CarTracker* CarTracker::tracker = nullptr;

void drawRect(Mat &img, cv::Rect rect, const char *str = "")
{
    cv::putText(img, str, cv::Point(rect.x-10,rect.y), 1, 2, Scalar(255,0,0));
    cv::rectangle(img, rect, Scalar(255,0,0));
}

CarTracker::CarTracker()
{
    videoSavePath = util.getValue("urlConfig", "VideoSavePath");
    imageSavePath = util.getValue("urlConfig", "ImageSavePath");

    shapeExtract = nullptr;
    colorExtract = nullptr;
    logoExtract = nullptr;
    plateExtract = nullptr;
    carDetector = nullptr;
    truckDetector = nullptr;
    motoShapeExtract = nullptr;
    motoDetector = nullptr;
    trainfo = nullptr;
}

CarTracker *CarTracker::getInstence()
{
    if(tracker == nullptr)
    {
        tracker = new CarTracker();
    }
    return tracker;
}

CarTracker::~CarTracker()
{
    if(shapeExtract != nullptr)
        delete shapeExtract;
    if(colorExtract != nullptr)
        delete colorExtract;
    if(logoExtract != nullptr)
        delete logoExtract;
    if(carDetector != nullptr)
        delete carDetector;
    if(plateExtract != nullptr)
        delete plateExtract;
    if(truckDetector != nullptr)
        delete truckDetector;
    if(motoShapeExtract != nullptr)
        delete motoShapeExtract;
    if(motoDetector != nullptr)
        delete motoDetector;
    if(trainfo != nullptr)
        delete trainfo;

    if(tracker != nullptr)
    {
        delete tracker;
        tracker = nullptr;
    }
}

std::vector<Prediction> CarTracker::getLogo(const cv::Mat &img, int top_k)
{
    if(logoExtract == nullptr)
    {
        qDebug() << "logoExtract init";

        logoExtract = new CarFeatureExtract("CarLogo");
    }

    return logoExtract->singleImageCarFeatureExtract(img, top_k);
}

std::vector<Prediction> CarTracker::getShape(const cv::Mat &img, int top_k)
{
    if(shapeExtract == nullptr)
    {
        qDebug() << "shapeExtract init";

        shapeExtract = new CarFeatureExtract("CarShape");
    }

    return shapeExtract->singleImageCarFeatureExtract(img, top_k);
}

std::vector<Prediction> CarTracker::getColor(const cv::Mat &img, int top_k)
{
    if(colorExtract == nullptr)
    {
        qDebug() << "colorExtract init";

        colorExtract = new CarFeatureExtract("CarColor");
    }

    return colorExtract->singleImageCarFeatureExtract(img, top_k);
}

std::vector<Prediction> CarTracker::getMotoShape(const Mat &img, int top_k)
{
    if(motoShapeExtract == nullptr)
    {
        qDebug() << "motoShapeExtract init";

        motoShapeExtract = new CarFeatureExtract("MotoShape");
    }


    return motoShapeExtract->singleImageCarFeatureExtract(img, top_k);
}

string CarTracker::getPlate(const Mat &img)
{
    if(plateExtract == nullptr)
    {
        qDebug() << "plateExtract init";

        const char *tagName = "CarPlate";
        string plateSvmTrainModel = util.getValue(tagName, "PlateSvmTrainModel").toStdString();
        string enTrainModel = util.getValue(tagName, "EnTrainModel").toStdString();
        string chTrainModel = util.getValue(tagName, "ChTrainModel").toStdString();

        plateExtract = new Lprs(plateSvmTrainModel, enTrainModel, chTrainModel);
    }
    return plateExtract->prosess(img);
}

float* CarTracker::getSiftFeature(Mat img)
{
    SiftDescriptorExtractor extractor;
    vector<KeyPoint> keypoints;
    cv::Mat des;
    float *data = new float[128];
    float *temp = data;
    int step = 10; // 10 pixels spacing between kp's

    for (int y = step; y < img.rows-step; y += step){
        for (int x = step; x < img.cols-step; x += step){
            // x,y,radius
            keypoints.push_back(KeyPoint(float(x), float(y), float(step)));
        }
    }

    extractor.compute(img, keypoints, des);

    for(int i = 0; i < des.cols; i++)
    {
        float max = 0.0;
        for(int j = 0; j < des.rows; j++)
        {
            max=fabs(des.at<float>(j,i)) > max?fabs(des.at<float>(j,i)):max;
        }
        *(temp++) = max;
    }

    return data;
}

void CarTracker::removeUselessCarHistCache(vector<CarHistCache> &chc)
{
    int size = chc.size();
    for (int i = 0; i < size; i++)
    {
        if (!chc[i].isUpdate && chc[i].positionChange)
        {
            chc.erase(chc.begin() + i);
            i--;
            size--;
        }
    }
}

void CarTracker::getHist(Mat image, float hist[], int count)
{
    if (image.channels() > 1)
        cvtColor(image, image, COLOR_BGR2GRAY);

    int histSize[] = { 256 };
    float granges[] = { 0, 256 };
    const float* ranges[] = { granges };
    MatND histND;
    int channels[] = { 0 };

    calcHist(&image, 1, channels, Mat(), histND, 1, histSize, ranges, true, false);

    for (int i = 0; i < count; i++)
        hist[i] = histND.at<float>(i, 0);
}


void CarTracker::initialCarHistCache(int dealType,string shape, string color, cv::VideoCapture &capture, vector<CarHistCache> &chc)
{
    Mat frame;

    while (capture.read(frame))
    {
        bool isPointFrame = false;
        vector<cv::Rect> &&cars = getCars(frame);

        for (vector<cv::Rect>::iterator it = cars.begin(); it != cars.end(); it++)
        {
            Mat dealCar = frame((*it)).clone();

            if (dealType == 1)
            {
                auto dcolor = getColor(frame(*it).clone(), 2);
                auto dshape = getShape(frame(*it).clone(), 2);

                if (compareColor(dcolor, color)
                    && compareShape(dshape, shape))
                {
                    CarHistCache subChc;
                    getHist(dealCar, subChc.backFrameCarHist, 256);
                    subChc.backFrameCarposition = (*it);
                    subChc.isUpdate = true;
                    subChc.num = 1;
                    subChc.positionChange = false;
                    chc.push_back(subChc);

                    isPointFrame = true;
                    drawRect(frame, (*it));
                }
            }
            else if (dealType == 2)
            {
                auto dshape = shapeExtract->singleImageCarFeatureExtract(frame(*it).clone());

                if (compareShape(dshape, shape))
                {
                    CarHistCache subChc;
                    getHist(dealCar, subChc.backFrameCarHist, 256);
                    subChc.backFrameCarposition = (*it);
                    subChc.isUpdate = true;
                    subChc.num = 1;
                    subChc.positionChange = false;
                    chc.push_back(subChc);

                    isPointFrame = true;
                    drawRect(frame, (*it));
                }
            }
            else if (dealType == 3)
            {
                auto dcolor = colorExtract->singleImageCarFeatureExtract(frame(*it).clone());

                if (compareColor(dcolor, color))
                {
                    CarHistCache subChc;
                    getHist(dealCar, subChc.backFrameCarHist, 256);
                    subChc.backFrameCarposition = (*it);
                    subChc.isUpdate = true;
                    subChc.num = 1;
                    subChc.positionChange = false;
                    chc.push_back(subChc);

                    isPointFrame = true;
                    drawRect(frame, (*it));
                }
            }
        }

        //cv::imshow("initialCarHistCache Extracted Frame", frame);

        if (isPointFrame)
        {
            break;
        }
    }
}

string CarTracker::carTrack(string videoFileName, string shape, string color, string logo, string plate)
{
    OutputStream video_st = { 0 };
    string outputVideoName;
    AVOutputFormat *fmt;
    AVFormatContext *oc;
    AVCodec *video_codec;
    int ret;
    int have_video = 0;
    int encode_video = 0;
    AVDictionary *opt = NULL;

    /* Initialize libavcodec, and register all codecs and formats. */
    av_register_all();

    outputVideoName = videoSavePath.toStdString() + "/" + QUuid::createUuid().toString().replace("}", "").replace("{", "").toStdString() + ".mp4";

    /* allocate the output media context */
    avformat_alloc_output_context2(&oc, NULL, NULL, outputVideoName.c_str());
    if (!oc) {
        printf("Could not deduce output format from file extension: using MPEG.\n");
        avformat_alloc_output_context2(&oc, NULL, "mpeg", outputVideoName.c_str());
    }
    if (!oc)
        return "";

    fmt = oc->oformat;

    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    vector<CarHistCache> chc;
    cv::VideoCapture capture(videoFileName);
    capture.open(videoFileName);

    if (!capture.isOpened())
    {
        qDebug() << "capture not opened";
        return "";
    }
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    /* Add the audio and video streams using the default format codecs
     * and initialize the codecs. */
    if (fmt->video_codec != AV_CODEC_ID_NONE) {
        add_stream(&video_st, oc, &video_codec, fmt->video_codec, capture.get(CV_CAP_PROP_FRAME_WIDTH), capture.get(CV_CAP_PROP_FRAME_HEIGHT));
        have_video = 1;
        encode_video = 1;
    }
    /* Now that all the parameters are set, we can open the audio and
     * video codecs and allocate the necessary encode buffers. */
    if (have_video)
        open_video(oc, video_codec, &video_st, opt);


    av_dump_format(oc, 0, outputVideoName.c_str(), 1);

    /* open the output file, if needed */
    if (!(fmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&oc->pb, outputVideoName.c_str(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            cout << "Could not open '%s': %s\n" + string(outputVideoName) << endl;
            return "";
        }
    }

    /* Write the stream header, if any. */
    ret = avformat_write_header(oc, &opt);
    if (ret < 0) {
        cout << "Error occurred when opening output file: %s\n" << endl;
        return "";
    }

    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    cv::Mat frame;
    cv::namedWindow("Extracted Frame");


    //根据给的车型，颜色参数，判断根据车型、颜色的组合筛选
    int dealType;           // 为1 车型、颜色 2 车型 3颜色
    if (!shape.empty() && !color.empty())
        dealType = 1;
    else if (!shape.empty())
        dealType = 2;
    else if (!color.empty())
        dealType = 3;
    else
    {
        qDebug() << "function parameter shape and color have no data ...";
        return "";
    }

    float currentFrameHist[256];  //当前帧的灰度直方图

    initialCarHistCache(dealType,shape, color, capture, chc);

    while (capture.read(frame))
    {
        imshow("readImage",frame);
        waitKey(1);

        for (int i = 0; i < chc.size(); i++)
            chc[i].isUpdate = false;

        vector<cv::Rect> &&cars = getCars(frame);

        for (vector<cv::Rect>::iterator it = cars.begin(); it != cars.end(); it++)
        {
            Mat dealCar = frame((*it)).clone();
            int left = (*it).x;
            int right = (*it).x + (*it).width;
            int down = (*it).y;
            int up = (*it).y + (*it).height;

            getHist(dealCar, currentFrameHist, 256);
            bool isInChc = false;

            if (dealType == 1)
            {
                auto dcolor = getColor(frame(*it).clone(), 2);
                auto dshape = getShape(frame(*it).clone(), 2);

                if (compareColor(dcolor, color) && compareShape(dshape, shape))
                {
                    for (int j = 0; j < chc.size(); j++)
                    {
                        if (compareCOSLike(chc[j].backFrameCarHist, currentFrameHist, 256))
                        {
                            chc[j].isUpdate = true;
                            chc[j].num++;
                            isInChc = true;

                            //当车位置不重合时，判断车移动了
                            int backLeft = chc[j].backFrameCarposition.x;
                            int backRight = chc[j].backFrameCarposition.x + chc[j].backFrameCarposition.width;
                            int backDown = chc[j].backFrameCarposition.y;
                            int backUp = chc[j].backFrameCarposition.y + chc[j].backFrameCarposition.height;

                            bool nonOverLap = left > backRight || right < backLeft || down > backUp || up < backDown;
                            if (nonOverLap)
                                chc[j].positionChange = true;
                        }
                    }

                    if (!isInChc)
                    {
                        CarHistCache subChc;
                        memcpy(subChc.backFrameCarHist, currentFrameHist, sizeof(currentFrameHist));
                        subChc.backFrameCarposition = (*it);
                        subChc.isUpdate = true;
                        subChc.num = 1;
                        subChc.positionChange = false;
                        chc.push_back(subChc);
                    }

                    drawRect(frame, (*it));
                }
            }
            else if (dealType == 2)
            {
                auto dshape = getShape(frame(*it).clone(), 2);

                if (compareShape(dshape, shape))
                {
                    for (int j = 0; j < chc.size(); j++)
                    {
                        if (compareCOSLike(chc[j].backFrameCarHist, currentFrameHist, 256))
                        {
                            chc[j].isUpdate = true;
                            chc[j].num++;
                            isInChc = true;

                            //当车位置不重合时，判断车移动了
                            int backLeft = chc[j].backFrameCarposition.x;
                            int backRight = chc[j].backFrameCarposition.x + chc[j].backFrameCarposition.width;
                            int backDown = chc[j].backFrameCarposition.y;
                            int backUp = chc[j].backFrameCarposition.y + chc[j].backFrameCarposition.height;

                            bool nonOverLap = left > backRight || right < backLeft || down > backUp || up < backDown;
                            if (nonOverLap)
                                chc[j].positionChange = true;
                        }
                    }

                    if (!isInChc)
                    {
                        CarHistCache subChc;
                        memcpy(subChc.backFrameCarHist, currentFrameHist, sizeof(currentFrameHist));
                        subChc.backFrameCarposition = (*it);
                        subChc.isUpdate = true;
                        subChc.num = 1;
                        subChc.positionChange = false;
                        chc.push_back(subChc);
                    }

                    drawRect(frame, (*it));
                }
            }
            else if (dealType == 3)
            {
                auto dcolor = getColor(frame(*it).clone(), 2);

                if (compareColor(dcolor, color))
                {
                    for (int j = 0; j < chc.size(); j++)
                    {
                        if (compareCOSLike(chc[j].backFrameCarHist, currentFrameHist, 256))
                        {
                            chc[j].isUpdate = true;
                            chc[j].num++;
                            isInChc = true;

                            //当车位置不重合时，判断车移动了
                            int backLeft = chc[j].backFrameCarposition.x;
                            int backRight = chc[j].backFrameCarposition.x + chc[j].backFrameCarposition.width;
                            int backDown = chc[j].backFrameCarposition.y;
                            int backUp = chc[j].backFrameCarposition.y + chc[j].backFrameCarposition.height;

                            bool nonOverLap = left > backRight || right < backLeft || down > backUp || up < backDown;
                            if (nonOverLap)
                                chc[j].positionChange = true;
                        }
                    }

                    if (!isInChc)
                    {
                        CarHistCache subChc;
                        memcpy(subChc.backFrameCarHist, currentFrameHist, sizeof(currentFrameHist));
                        subChc.backFrameCarposition = (*it);
                        subChc.isUpdate = true;
                        subChc.num = 1;
                        subChc.positionChange = false;
                        chc.push_back(subChc);
                    }

                    drawRect(frame, (*it));
                }
            }
        }

        bool isWrite = false;
        for (int i = 0; i < chc.size(); i++)
        {
            if (chc[i].isUpdate && chc[i].num < 13)
            {
                isWrite = true;
                break;
            }
        }

        if (isWrite)
        {
            imshow("wirteImage",frame);
            waitKey(1);
            write_video_frame(oc, &video_st, frame);
        }

        removeUselessCarHistCache(chc);
    }

    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    /* Write the trailer, if any. The trailer must be written before you
     * close the CodecContexts open when you wrote the header; otherwise
     * av_write_trailer() may try to use memory that was freed on
     * av_codec_close(). */
    av_write_trailer(oc);

    close_stream(oc, &video_st);

    if (!(fmt->flags & AVFMT_NOFILE))        /* Close the output file. */
        avio_closep(&oc->pb);

    /* free the stream */
    avformat_free_context(oc);

    capture.release();

    return outputVideoName;
}

string CarTracker::motoTrack(string videoFileName, string shape)
{
    cv::VideoCapture capture(videoFileName);

    if (!capture.isOpened())
    {
        qDebug() << "capture not opened";
        return "";
    }

    string outputVideoName = videoSavePath.toStdString() + "/" + "output_local.avi";
    VideoWriter writer;

    int frameRate = 25;
    Size frameSize;

    if (!writer.isOpened())
    {
        frameSize.width  = capture.get(CV_CAP_PROP_FRAME_WIDTH);
        frameSize.height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);

        if (!writer.open(outputVideoName, CV_FOURCC('D','I','V','X') , frameRate, frameSize, true))
        {
            qDebug() << "open writer error...";
            return "";
        }
    }

    double rate = capture.get(CV_CAP_PROP_FPS);
    cv::Mat frame;
    cv::namedWindow("Extracted Frame");
    int delay = rate;

    while (capture.read(frame))
    {
        bool isPointFrame = false;
        vector<cv::Rect> &&motos = getMotos(frame);

        for(vector<cv::Rect>::iterator it = motos.begin(); it != motos.end(); it++)
        {
            auto dshape = getMotoShape(frame(*it).clone(), 2);

            if(compareShape(dshape, shape))
            {
                isPointFrame = true;
                drawRect(frame, (*it));
            }
        }

        cv::imshow("Extracted Frame", frame);

        if(isPointFrame)
        {
            writer.write(frame);
        }

        if (cv::waitKey(delay) >= 0)
            break;

    }

    writer.release();
    capture.release();

    return outputVideoName;
}

void CarTracker::truckTrack(string videoFileName, string startTime, string channelCode)
{
    cv::VideoCapture capture(videoFileName);
    QVector<CarInfo *> matchList;

    if (!capture.isOpened())
    {
        qDebug() << "capture not opened";
        return;
    }

    float rate = capture.get(CV_CAP_PROP_FPS);
    float usPerFrame = 1000/rate;

    cv::Mat frame;
    QDateTime time = QDateTime::fromTime_t(atoi(startTime.c_str()));
    int cross = 3;
    int num = 0;

    while (capture.read(frame))
    {
        time = time.addMSecs(usPerFrame);

        if((num++)%cross != 0)
        {
            continue;
        }

        getTrucks(frame, QString::number(time.toTime_t()), QString(channelCode.c_str()), matchList);

        excuteMatchList(matchList);
    }

    for(int i = 0; i < matchList.size(); i++)
        delete matchList.at(i);

    capture.release();
}

void CarTracker::taxiTrack(string videoFileName, string startTime, string channelCode)
{
    cv::VideoCapture capture(videoFileName);
    QVector<CarInfo *> matchList;

    if (!capture.isOpened())
    {
        qDebug() << "capture not opened";
        return;
    }

    float rate = capture.get(CV_CAP_PROP_FPS);
    float usPerFrame = 1000/rate;
    qDebug() << "uspf:" << usPerFrame;

    cv::Mat frame;
    QDateTime time = QDateTime::fromTime_t(atoi(startTime.c_str()));
    int cross = 3;
    int num = 0;

    while (capture.read(frame))
    {
        time = time.addMSecs(usPerFrame);

        if((num++)%cross != 0)
        {
            continue;
        }

        vector<cv::Rect> &&cars = getCars(frame);
        bool isPointFrame = false;

        for(vector<cv::Rect>::iterator it = cars.begin(); it != cars.end(); it++)
        {
            if(!checkMatchList(frame(*it), matchList) )
            {
                cv::Mat mat = frame(*it).clone();

                int width = (*it).width;
                int height = (*it).height;

                if(width > height)
                    cv::copyMakeBorder(mat, mat, (width - height)/2,  (width - height)/2, 0, 0, cv::BORDER_CONSTANT, Scalar(0,0,0) );
                else if(height > width)
                    cv::copyMakeBorder(mat, mat, 0, 0, (height - width)/2,  (height - width)/2, cv::BORDER_CONSTANT, Scalar(0,0,0) );

                auto dcolor = getColor(mat, 2);
                auto dshape = getShape(mat, 2);

                if(dcolor.at(0).first == "green" && (dshape.at(0).first == "sedan" || dshape.at(0).first == "hatchback"))
                {
                    cv::rectangle(frame, *it, Scalar(0, 0, 255), 2);
                    cv::putText(frame,dcolor.at(0).first+"_"+dshape.at(0).first,cv::Point((*it).x-5,(*it).y),0,1,Scalar(0,0,255),2);
                    isPointFrame = true;
                }

                CarInfo *info = new CarInfo;
                info->boundingBox = *it;
                info->matchNums = 0;
                info->feature = getHistFeature(frame(*it));
                info->mat = frame(*it).clone();

                matchList.push_back(info);

                cv::imshow("not match",frame(*it));
                cv::waitKey();
            }
            else
            {
                qDebug() << "------------------------match---------------------";
            }
        }

        excuteMatchList(matchList);

        if(isPointFrame)
        {
            QString imageUrl = imageSavePath + "/" + QUuid::createUuid().toString().replace("{", "").replace("}", "") + ".jpg";
            cv::imwrite(imageUrl.toStdString(), frame);
            QString sql = "insert into carPic_info(channel_id,carPic_time,carPic_url,carPic_type) values('%1','%2','%3','%4')";

            dbManager.execQuery(sql.arg(QString(channelCode.c_str()) ).arg(QString::number(time.toTime_t()) ).arg(imageUrl).arg("taxi"));
        }
    }

    for(int i = 0; i < matchList.size(); i++)
        delete matchList.at(i);

    capture.release();
}

void CarTracker::areaCarTrack(string videoFileName, string startTime, string channelCode, string areas)
{
    QStringList t = QString(areas.c_str()).split(",");
    cout<< t.size() <<endl;
    vector<std::pair<cv::Point2f,cv::Point2f> > rectAreas;

    for(int i = 0; i < t.size()/4; i++)
    {
        rectAreas.push_back(getPoints(t.at(i*4).toFloat(),
                                      t.at(i*4+1).toFloat(),
                                      t.at(i*4+2).toFloat(),
                                      t.at(i*4+3).toFloat()));
    }

    cv::VideoCapture capture(videoFileName);

    if (!capture.isOpened())
    {
        qDebug() << "capture not opened";
        return;
    }

    float rate = capture.get(CV_CAP_PROP_FPS);
    float usPerFrame = 1000/rate;

    cv::Mat frame;
    QDateTime time = QDateTime::fromTime_t(atoi(startTime.c_str()));
    int cross = 20;
    int num = 0;

    while (capture.read(frame))
    {
        time = time.addMSecs(usPerFrame);
        bool isPointFrame = false;

        if((num++)%cross != 0)
        {
            continue;
        }

        for(int i = 0; i < rectAreas.size(); i++)
        {
            int tx1 = rectAreas.at(i).first.x*frame.cols;
            int ty1 = rectAreas.at(i).first.y*frame.rows;
            int tx2 = rectAreas.at(i).second.x*frame.cols;
            int ty2 = rectAreas.at(i).second.y*frame.rows;

            cv::Rect tRect(tx1, ty1, tx2-tx1, ty2-ty1);
            cv::rectangle(frame, tRect, Scalar(255, 0, 0), 2);
        }

        vector<cv::Rect> &&cars = getCars(frame);

        for(vector<cv::Rect>::iterator it = cars.begin(); it != cars.end(); it++)
        {
            if(inRect(*it,rectAreas, frame.cols, frame.rows))
            {
                cv::rectangle(frame, *it, Scalar(0, 0, 255), 2);
                isPointFrame = true;
            }
        }

        if(isPointFrame)
        {
            QString imageUrl = imageSavePath + "/" + QUuid::createUuid().toString().replace("{", "").replace("}", "") + ".jpg";
            cv::imwrite(imageUrl.toStdString(),frame);
            QString sql = "insert into carPic_info(channel_id,carPic_time,carPic_url,carPic_type) values('%1','%2','%3','%4')";

            dbManager.execQuery(sql.arg(QString(channelCode.c_str()) ).arg(QString::number(time.toTime_t()) ).arg(imageUrl).arg("areacar"));
        }
    }

    capture.release();
}

string CarTracker::trafficStatistics(string videoFileName, string areas)
{
    if(carDetector == nullptr)
    {
        qDebug() << "car detector init";

        const char* tagName = "CarDetect";
        string model_file   = util.getValue(tagName, "modelFilePath").toStdString();
        string trained_file = util.getValue(tagName, "trainedFilePath").toStdString();
        string mean_file    = util.getValue(tagName, "meanFilePath").toStdString();
        string mean_value   = util.getValue(tagName, "meanValue").toStdString();
        carThreshold = util.getValue(tagName, "ConfidenceThreshold").toFloat();

        carDetector = new Detector(model_file, trained_file, mean_file, mean_value);
    }

    if(trainfo == nullptr)
    {
        trainfo = new Trainfo();
    }

    cv::VideoCapture capture(videoFileName);

    if (!capture.isOpened())
    {
        qDebug() << "capture not opened";
        return "";
    }

    int width  = capture.get(CV_CAP_PROP_FRAME_WIDTH);
    int height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);


    QStringList t = QString(areas.c_str()).split(",");

    std::vector<vector<float> > rectboxes;

    for(int i = 0; i < t.size()/4; i++)
    {
        auto points = getPoints(t.at(i*4).toFloat(),
                                t.at(i*4+1).toFloat(),
                                t.at(i*4+2).toFloat(),
                                t.at(i*4+3).toFloat());
        cv::Point2f p1 = points.first;
        cv::Point2f p2 = points.second;

        std::vector<float> rectboxtrans;
        rectboxtrans.push_back(0);
        rectboxtrans.push_back(p1.x*width);
        rectboxtrans.push_back(p1.y*height);
        rectboxtrans.push_back(p2.x*width);
        rectboxtrans.push_back(p2.y*height);

        rectboxes.push_back(rectboxtrans);
    }

    //    std::vector<float> rectboxtrans;
    //    rectboxtrans.push_back(0);
    //    rectboxtrans.push_back(800);
    //    rectboxtrans.push_back(500);
    //    rectboxtrans.push_back(800 + 260);
    //    rectboxtrans.push_back(500 + 200);

    //    std::vector<float> rectboxtrans_1;
    //    rectboxtrans_1.push_back(0);
    //    rectboxtrans_1.push_back(100);
    //    rectboxtrans_1.push_back(500);
    //    rectboxtrans_1.push_back(100 + 260);
    //    rectboxtrans_1.push_back(500 + 200);

    //    std::vector<float> rectboxtrans_2;
    //    rectboxtrans_2.push_back(0);
    //    rectboxtrans_2.push_back(460);
    //    rectboxtrans_2.push_back(500);
    //    rectboxtrans_2.push_back(460 + 260);
    //    rectboxtrans_2.push_back(500 + 200);

    //    std::vector<vector<float> > rectboxes;
    //    rectboxes.push_back(rectboxtrans);
    //    rectboxes.push_back(rectboxtrans_1);
    //    rectboxes.push_back(rectboxtrans_2);

    string outputVideoName = videoSavePath.toStdString() + "/" + "output_local.avi";
    //路口画框
    trainfo->CrossingDemo(carDetector, videoFileName, outputVideoName, rectboxes);

    return outputVideoName;
}

pair<Point2f, Point2f> CarTracker::getPoints(float a1, float a2, float b1, float b2)
{
    float temp;

    if(a1 > b1)
    {
        temp = a1;
        a1 = b1;
        b1 = temp;
    }

    if(a2 > b2)
    {
        temp = a2;
        a2 = b2;
        b2 = temp;
    }


    std::cout << a1 << " " << a2 << "," << b1 << " " << b2 << "**************************************" <<endl;
    return std::make_pair(cv::Point2f(a1, a2), cv::Point2f(b1, b2));
}

bool CarTracker::inRect(Rect rect, vector<pair<Point2f, Point2f> > rectAreas, int cols, int rows)
{
    for(int i = 0; i < rectAreas.size(); i++)
    {
        int tx1 = rectAreas.at(i).first.x*cols;
        int ty1 = rectAreas.at(i).first.y*rows;
        int tx2 = rectAreas.at(i).second.x*cols;
        int ty2 = rectAreas.at(i).second.y*rows;
        cv::Rect tRect(tx1, ty1, tx2-tx1, ty2-ty1);

        int x0 = std::max(rect.x, tRect.x);
        int x1 = std::min(rect.x + rect.width, tRect.x + tRect.width);
        int y0 = std::max(rect.y, tRect.y);
        int y1 = std::min(rect.y + rect.height, tRect.y + tRect.height);

        if ((x0 < x1) && (y0 < y1))
            return true;
    }

    return false;
}

QString CarTracker::getImageSavePath()
{
    return imageSavePath;
}

vector<Rect> CarTracker::getCars(Mat &img)
{
    vector<Rect> results;

    if(carDetector == nullptr)
    {
        qDebug() << "car detector init";

        const char* tagName = "CarDetect";
        string model_file   = util.getValue(tagName, "modelFilePath").toStdString();
        string trained_file = util.getValue(tagName, "trainedFilePath").toStdString();
        string mean_file    = util.getValue(tagName, "meanFilePath").toStdString();
        string mean_value   = util.getValue(tagName, "meanValue").toStdString();
        carThreshold = util.getValue(tagName, "ConfidenceThreshold").toFloat();

        carDetector = new Detector(model_file, trained_file, mean_file, mean_value);
    }

    int pad=10;
    std::vector<vector<float> > detections = carDetector->Detect(img);

    for (int i = 0; i < detections.size(); ++i) {
        const vector<float> &d = detections[i];
        if (d[0] >= carThreshold) {

            int x=(d[1]-pad > 0)?(d[1]-pad):0;
            int y=(d[2]-pad > 0)?(d[2]-pad):0;
            int width=(d[3]+pad < img.cols)?(d[3]-d[1]+pad):(img.cols-d[1]);
            int height=(d[4]+pad < img.rows)?(d[4]-d[2]+pad):(img.rows-d[2]);

            cv::Rect rect(x, y, width, height);
            results.push_back(rect);
        }
    }

    return results;
}

vector<Rect> CarTracker::getMotos(Mat &img)
{
    vector<Rect> results;

    if(motoDetector == nullptr)
    {
        qDebug() << "moto detector init";

        const char* tagName = "MotoDetect";
        string model_file   = util.getValue(tagName, "modelFilePath").toStdString();
        string trained_file = util.getValue(tagName, "trainedFilePath").toStdString();
        string mean_file    = util.getValue(tagName, "meanFilePath").toStdString();
        string mean_value   = util.getValue(tagName, "meanValue").toStdString();
        motoThreshold = util.getValue(tagName, "ConfidenceThreshold").toFloat();

        motoDetector = new Detector(model_file, trained_file, mean_file, mean_value);
    }

    int pad=10;
    std::vector<vector<float> > detections = motoDetector->Detect(img);

    for (int i = 0; i < detections.size(); ++i) {
        const vector<float> &d = detections[i];
        if (d[0] >= motoThreshold) {

            int x=(d[1]-pad > 0)?(d[1]-pad):0;
            int y=(d[2]-pad > 0)?(d[2]-pad):0;
            int width=(d[3]+pad < img.cols)?(d[3]-d[1]+pad):(img.cols-d[1]);
            int height=(d[4]+pad < img.rows)?(d[4]-d[2]+pad):(img.rows-d[2]);

            cv::Rect rect(x, y, width, height);
            results.push_back(rect);
        }
    }

    return results;
}

void CarTracker::getTrucks(Mat &img, QString time, QString channelCode, QVector<CarInfo *> &matchList)
{
    bool isPointFrame = false;

    if(truckDetector == nullptr)
    {
        qDebug() << "truck detector init";

        const char* tagName = "TruckDetect";
        string model_file   = util.getValue(tagName, "modelFilePath").toStdString();
        string trained_file = util.getValue(tagName, "trainedFilePath").toStdString();
        string mean_file    = util.getValue(tagName, "meanFilePath").toStdString();
        string mean_value   = util.getValue(tagName, "meanValue").toStdString();
        truckThreshold = util.getValue(tagName, "ConfidenceThreshold").toFloat();

        qDebug() << model_file.c_str() << trained_file.c_str() << mean_value.c_str() << mean_file.c_str() << truckThreshold << "ssssssss";

        truckDetector = new Detector(model_file, trained_file, mean_file, mean_value);
    }

    int pad=10;

    std::vector<vector<float> > detections = truckDetector->Detect(img);

    for (int i = 0; i < detections.size(); ++i) {
        const vector<float>& d = detections[i];
        if (d[0] >= truckThreshold && d[5] == 1) {

            int x=(d[1]-pad > 0)?(d[1]-pad):0;
            int y=(d[2]-pad > 0)?(d[2]-pad):0;
            int width=(d[3]+pad < img.cols)?(d[3]-d[1]+pad):(img.cols-d[1]);
            int height=(d[4]+pad < img.rows)?(d[4]-d[2]+pad):(img.rows-d[2]);

            cv::Rect rect(x, y, width, height);

            if(!checkMatchList(img(rect), matchList) )
            {
                cv::rectangle(img, rect, Scalar(0, 0, 255), 2);
                isPointFrame = true;

                CarInfo *info = new CarInfo;
                info->boundingBox = rect;
                info->matchNums = 0;
                info->feature = getHistFeature(img(rect));
                info->mat = img(rect).clone();

                matchList.push_back(info);

                cv::imshow("not match",img(rect));
                cv::waitKey();
            }
            else
            {
                qDebug() << "------------------------match---------------------";
            }
        }
    }


    if(isPointFrame)
    {
        QString imageUrl = imageSavePath + "/" + QUuid::createUuid().toString().replace("{", "").replace("}", "") + ".jpg";
        cv::imwrite(imageUrl.toStdString(), img);
        QString sql = "insert into carPic_info(channel_id,carPic_time,carPic_url,carPic_type) values('%1',%2,'%3','%4')";

        dbManager.execQuery(sql.arg(channelCode).arg(time).arg(imageUrl).arg("truck"));
    }

}

bool CarTracker::compareCOSLike(float *t1, float *t2, int count)
{
    float dotSum = 0.0f;
    float modeT1 = 0.0f, modeT2 = 0.0f;
    for(int i = 0; i < count; i++)
    {
        dotSum += t1[i]*t2[i];
        modeT1 += t1[i]*t1[i];
        modeT2 += t2[i]*t2[i];
    }
    if(modeT1 == 0 || modeT2 == 0)
        return false;
    else
    {
        double d = acos(dotSum/(sqrt(fabs(modeT1) )*sqrt(fabs(modeT2) ) ) );
        std::cout << d << std::endl;
        return ( d <= 0.2);
    }
}

bool CarTracker::compareShape(std::vector<Prediction> &result,string shape)
{
    for(int i = 0; i < 2; i++)
    {
        if(result.at(i).first == shape)
            return true;

        if(result.at(i).second > 0.5)
            break;
    }

    return false;
}

bool CarTracker::compareColor(std::vector<Prediction> &result, string color)
{
    for(int i = 0; i < 2; i++)
    {
        if(result.at(i).first == color)
            return true;

        if(result.at(i).second > 0.5)
            break;
    }

    return false;
}

float* CarTracker::getHistFeature(Mat image)
{
    if (image.channels() > 1)
        cvtColor(image, image, COLOR_BGR2GRAY);

    cv::Mat histogram;
    const int histSize = 256;
    float range[] = {0, 255};
    const float *ranges[] = {range};
    const int channels = 0;

    cv::calcHist(&image, 1, &channels, cv::Mat(), histogram, 1, &histSize, &ranges[0], true, false);

    float *h = (float*)histogram.data;
    float *hh = new float[256];

    if (h) {
        for (int i = 0; i < 256; ++i) {
            hh[i] = h[i];
        }
    }

    return hh;
}

bool CarTracker::checkMatchList(cv::Mat mat, QVector<CarInfo *> &matchList)
{
    if(matchList.size() == 0)
    {
        std::cout << "empty list" << std::endl;
        return false;
    }

    float *feature = getHistFeature(mat);

    for(int i =0; i < matchList.size(); i++)
    {
        if(compareCOSLike(matchList.at(i)->feature, feature, 256))
        {
            cv::imshow("matched",matchList.at(i)->mat);
            cv::imshow("src",mat);
            cv::waitKey();

            delete matchList.at(i)->feature;
            matchList.at(i)->matchNums = -1;
            matchList.at(i)->feature = feature;

            return true;
        }
    }

    delete feature;

    return false;
}

void CarTracker::excuteMatchList(QVector<CarInfo *> &matchList)
{
    for(QVector<CarInfo *>::Iterator it = matchList.begin(); it != matchList.end(); )
    {
        if((++((*it)->matchNums) ) == 5)
        {
            delete *it;
            it = matchList.erase(it);

            continue;
        }

        it++;
    }
}
