#include "settingutil.h"
#include <QtCore/QCoreApplication>
#include "CarFeature/carfeatureextract.h"
#include "CarDetect/cardetector.h"

SettingUtil util("/home/zg/traffic/QtProject/TestClassify/config.ini");

void testClassify(QString fileDir, QString tag, bool makeBorder = false, int top_k = 5)
{
    CarFeatureExtract extractor(tag);
    QDir dir(fileDir);

    QStringList filter;
    filter << "*.jpg" << "*.jpeg" << "*.png";
    QFileInfoList ls = dir.entryInfoList(filter, QDir::Files);

    foreach(QFileInfo info,ls)
    {
        qDebug() << info.filePath();

        cv::Mat mat = cv::imread(info.filePath().toStdString().c_str());

        if(makeBorder)
        {
            int width = mat.cols;
            int height = mat.rows;

            if(width > height)
                cv::copyMakeBorder(mat, mat, (width - height)/2,  (width - height)/2, 0, 0, cv::BORDER_CONSTANT, cv::Scalar(0,0,0) );
            else if(height > width)
                cv::copyMakeBorder(mat, mat, 0, 0, (height - width)/2,  (height - width)/2, cv::BORDER_CONSTANT, cv::Scalar(0,0,0) );
        }

        cv::imshow("dd", mat);
        cv::moveWindow("dd",500,500);

        auto result = extractor.singleImageCarFeatureExtract(mat, top_k);

        for(int i = 0; i< result.size(); i++)
        {
            qDebug() << result.at(i).first.c_str() << "--" << result.at(i).second;
        }

        cv::waitKey();
    }
}


void testCarAllFeature(QString fileDir, int top_k = 5)
{
    CarFeatureExtract shapeExtractor("CarShape");
    CarFeatureExtract colorExtractor("CarColor");
    QDir dir(fileDir);

    QStringList filter;
    filter << "*.jpg" << "*.jpeg" << "*.png";
    QFileInfoList ls = dir.entryInfoList(filter, QDir::Files);

    foreach(QFileInfo info,ls)
    {
        qDebug() << info.filePath();

        cv::Mat mat = cv::imread(info.filePath().toStdString().c_str());
        cv::Mat temp = mat.clone();


        int width = mat.cols;
        int height = mat.rows;

        if(width > height)
            cv::copyMakeBorder(mat, mat, (width - height)/2,  (width - height)/2, 0, 0, cv::BORDER_CONSTANT, cv::Scalar(0,0,0) );
        else if(height > width)
            cv::copyMakeBorder(mat, mat, 0, 0, (height - width)/2,  (height - width)/2, cv::BORDER_CONSTANT, cv::Scalar(0,0,0) );


        cv::imshow("dd", mat);
        cv::moveWindow("dd",500,500);

        auto shapeResult = shapeExtractor.singleImageCarFeatureExtract(mat, top_k);
        auto colorResult = colorExtractor.singleImageCarFeatureExtract(temp, top_k);

        for(int i = 0; i< shapeResult.size(); i++)
        {
            qDebug() << shapeResult.at(i).first.c_str() << "--" << shapeResult.at(i).second;
        }

        qDebug() << "";

        for(int i = 0; i< colorResult.size(); i++)
        {
            qDebug() << colorResult.at(i).first.c_str() << "--" << colorResult.at(i).second;
        }

        cv::waitKey();
    }
}

void testDetect(QString fileName, QString tag)
{
    string model_file   = util.getValue(tag, "modelFilePath").toStdString();
    string trained_file = util.getValue(tag, "trainedFilePath").toStdString();
    string mean_file    = util.getValue(tag, "meanFilePath").toStdString();
    string mean_value   = util.getValue(tag, "meanValue").toStdString();
    float confidenceThreshold = util.getValue(tag, "ConfidenceThreshold").toFloat();

    Detector detector(model_file, trained_file, mean_file, mean_value);

    cv::VideoCapture capture(fileName.toStdString().c_str());
    if (!capture.isOpened())
    {
        std::cout<<"fail to open"<<std::endl;
        return;
    }


    std::cout<<"frameCount:"<<capture.get(CV_CAP_PROP_FRAME_COUNT);
    cv::Mat frame;
    while (capture.read(frame))
    {
        std::vector<vector<float> > detections = detector.Detect(frame);

        for (int i = 0; i < detections.size(); ++i) {
            const vector<float>& d = detections[i];
            if (d[0] >= confidenceThreshold) {
                cv::Rect rect(d[1], d[2], (d[3] - d[1]), (d[4] - d[2]));
                cv::rectangle(frame, rect, cvScalar(0, 255, 0));
                //cv::imwrite(("/home/zg/1T/test/"+QUuid::createUuid().toString()+".jpg").toStdString(),frame(rect));
            }
        }

        cv::imshow("result", frame);
        cv::waitKey();
    }
    capture.release();
}

void testDetectAndClassify(QString fileName, QString detectTag, QString classifyTag1, bool makeBorder1 = false, QString classifyTag2 = "",bool makeBorder2 = false)
{
    CarFeatureExtract extractor1(classifyTag1);
    CarFeatureExtract extractor2(classifyTag2);

    string model_file   = util.getValue(detectTag, "modelFilePath").toStdString();
    string trained_file = util.getValue(detectTag, "trainedFilePath").toStdString();
    string mean_file    = util.getValue(detectTag, "meanFilePath").toStdString();
    string mean_value   = util.getValue(detectTag, "meanValue").toStdString();
    float confidenceThreshold = util.getValue(detectTag, "ConfidenceThreshold").toFloat();

    Detector detector(model_file, trained_file, mean_file, mean_value);

    cv::VideoCapture capture(fileName.toStdString().c_str());
    if (!capture.isOpened())
    {
        std::cout<<"fail to open"<<std::endl;
        return;
    }


    std::cout<<"frameCount:"<<capture.get(CV_CAP_PROP_FRAME_COUNT);
    cv::Mat frame;
    int cross = 5;
    int frameNum = 0;

    while (capture.read(frame))
    {
        if((frameNum++)%cross != 0)
            continue;

        std::vector<vector<float> > detections = detector.Detect(frame);

        for (int i = 0; i < detections.size(); ++i) {
            const vector<float>& d = detections[i];
            if (d[0] >= confidenceThreshold) {
                cv::Rect rect(d[1], d[2], (d[3] - d[1]), (d[4] - d[2]));

                cv::Mat mat = frame(rect);
                cv::Mat temp = mat.clone();

                if(makeBorder1)
                {
                    int width = mat.cols;
                    int height = mat.rows;

                    if(width > height)
                        cv::copyMakeBorder(mat, mat, (width - height)/2,  (width - height)/2, 0, 0, cv::BORDER_CONSTANT, cv::Scalar(0,0,0) );
                    else if(height > width)
                        cv::copyMakeBorder(mat, mat, 0, 0, (height - width)/2,  (height - width)/2, cv::BORDER_CONSTANT, cv::Scalar(0,0,0) );
                }

                if(makeBorder2)
                {
                    int width = temp.cols;
                    int height = temp.rows;

                    if(width > height)
                        cv::copyMakeBorder(temp, temp, (width - height)/2,  (width - height)/2, 0, 0, cv::BORDER_CONSTANT, cv::Scalar(0,0,0) );
                    else if(height > width)
                        cv::copyMakeBorder(temp, temp, 0, 0, (height - width)/2,  (height - width)/2, cv::BORDER_CONSTANT, cv::Scalar(0,0,0) );
                }

                string txt = "";
                auto result1 = extractor1.singleImageCarFeatureExtract(mat, 1);

                for(int i = 0; i< result1.size(); i++)
                {
                    txt += result1.at(i).first;
                }

                qDebug() << "";

                if(classifyTag2 != "")
                {
                    auto result2 = extractor2.singleImageCarFeatureExtract(temp, 1);
                    for(int i = 0; i< result2.size(); i++)
                    {
                        txt += " " + result2.at(i).first;
                    }
                }


                cv::putText(frame,txt,cv::Point(rect.x-5,rect.y),0,1,Scalar(0,0,255),2);
                cv::rectangle(frame, rect, cvScalar(0, 255, 0));
            }
        }

        cv::imshow("result", frame);
        cv::waitKey(1);
    }
    capture.release();
}

typedef struct CarInfo
{
    float *feature;
    cv::Rect boundingBox;
    int matchNums;
    cv::Mat mat;

    ~CarInfo()
    {
        delete feature;
    }
}CarInfo;

bool compareCOSLike(float *t1, float *t2, int count)
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

#include "opencv2/opencv.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/ml/ml.hpp>
#include <QDateTime>
#include"cartracker.h"

vector<Rect> getCars(Mat &img)
{
    vector<Rect> results;

    static float carThreshold;
    static Detector *carDetector = nullptr;
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

float* getHistFeature(Mat image)
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

bool checkMatchList(cv::Mat mat, QVector<CarInfo *> &matchList)
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

void excuteMatchList(QVector<CarInfo *> &matchList)
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

void taxiTrack(string videoFileName, string startTime, string channelCode)
{
    CarFeatureExtract shapeExtractor("CarShape");
    CarFeatureExtract colorExtractor("CarColor");
    QVector<CarInfo *> matchList;

    cv::VideoCapture capture(videoFileName);

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

                auto dcolor = colorExtractor.singleImageCarFeatureExtract(mat,5);
                auto dshape = shapeExtractor.singleImageCarFeatureExtract(mat,5);

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


                cv::imshow("not match",frame(*it));
                cv::waitKey();
                matchList.push_back(info);
            }
            else
            {
                std::cout << "------------------------match---------------------" <<std::endl;
            }
        }

        excuteMatchList(matchList);

//        if(isPointFrame)
//        {
//            QString imageUrl = imageSavePath + "/" + QUuid::createUuid().toString().replace("{", "").replace("}", "") + ".jpg";
//            cv::imwrite(imageUrl.toStdString(), frame);
//            QString sql = "insert into carPic_info(channel_id,carPic_time,carPic_url,carPic_type) values('%1','%2','%3','%4')";

//            dbManager.execQuery(sql.arg(QString(channelCode.c_str()) ).arg(QString::number(time.toTime_t()) ).arg(imageUrl).arg("taxi"));
//        }
    }

    capture.release();
}

void getTrucks(Mat &img, QString time, QString channelCode, QVector<CarInfo *> &matchList)
{
    bool isPointFrame = false;

    static float truckThreshold;
    static Detector *truckDetector = nullptr;

    if(truckDetector == nullptr)
    {
        qDebug() << "truck detector init";

        const char* tagName = "TruckDetect";
        string model_file   = util.getValue(tagName, "modelFilePath").toStdString();
        string trained_file = util.getValue(tagName, "trainedFilePath").toStdString();
        string mean_file    = util.getValue(tagName, "meanFilePath").toStdString();
        string mean_value   = util.getValue(tagName, "meanValue").toStdString();
        truckThreshold = util.getValue(tagName, "ConfidenceThreshold").toFloat();

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
                CarInfo *info = new CarInfo;
                info->boundingBox = rect;
                info->matchNums = 0;
                info->feature = getHistFeature(img(rect));
                info->mat = img(rect).clone();


                cv::imshow("not match",img(rect));
                cv::waitKey();
                matchList.push_back(info);

                cv::rectangle(img, rect, Scalar(0, 0, 255), 2);
                isPointFrame = true;
            }
            else
            {
                qDebug() << "------------------------match---------------------";
            }
        }
    }


//    if(isPointFrame)
//    {
//        QString imageUrl = imageSavePath + "/" + QUuid::createUuid().toString().replace("{", "").replace("}", "") + ".jpg";
//        cv::imwrite(imageUrl.toStdString(), img);
//        QString sql = "insert into carPic_info(channel_id,carPic_time,carPic_url,carPic_type) values('%1',%2,'%3','%4')";

//        dbManager.execQuery(sql.arg(channelCode).arg(time).arg(imageUrl).arg("truck"));
//    }
}

void truckTrack(string videoFileName, string startTime, string channelCode)
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
}

void close_stream(AVFormatContext *oc, OutputStream *ost)
{
    avcodec_free_context(&ost->enc);
    av_frame_free(&ost->frame);
    av_frame_free(&ost->tmp_frame);
    sws_freeContext(ost->sws_ctx);
    swr_free(&ost->swr_ctx);
}

/**************************************************************/
/* media file output */

int main(int argc, char **argv)
{
    OutputStream video_st = { 0 };
    const char *filename;
    AVOutputFormat *fmt;
    AVFormatContext *oc;
    AVCodec *video_codec;
    int ret;
    int have_video = 0;
    int encode_video = 0;
    AVDictionary *opt = NULL;

    /* Initialize libavcodec, and register all codecs and formats. */
    av_register_all();

    filename = "./ff.mp4";

    /* allocate the output media context */
    avformat_alloc_output_context2(&oc, NULL, NULL, filename);
    if (!oc) {
        printf("Could not deduce output format from file extension: using MPEG.\n");
        avformat_alloc_output_context2(&oc, NULL, "mpeg", filename);
    }
    if (!oc)
        return 1;

    fmt = oc->oformat;


    cv::VideoCapture capture("/home/zg/test.avi");
    if (!capture.isOpened())
    {
        std::cout<<"fail to open"<<std::endl;
        exit(1);
    }

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


    av_dump_format(oc, 0, filename, 1);

    /* open the output file, if needed */
    if (!(fmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&oc->pb, filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            cout << "Could not open '%s': %s\n" + string(filename) << endl;
            return 1;
        }
    }

    /* Write the stream header, if any. */
    ret = avformat_write_header(oc, &opt);
    if (ret < 0) {
        cout << "Error occurred when opening output file: %s\n" << endl;
        return 1;
    }

    cv::Mat frame;

    while (capture.read(frame))
    {
        write_video_frame(oc, &video_st, frame);
    }

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

    return 0;
}


//int main(int argc, char *argv[])
//{
//    QCoreApplication a(argc, argv);

//    ::google::InitGoogleLogging("");

//    //------------------CarShape Classify (by change parameter tag)
//    //testClassify("/home/zg/1T/samples/0","MotoShape",true);

//    //------------------CarColor Classify (by change parameter tag)
//    //testClassify("/home/zg/1T/samples/0","CarColor");

//    //------------------MotoShape Classify vaild
//    //testClassify("/home/zg/1T/test","MotoShape");

//    //------------------CarShape and CarColor Classify
//    //testCarAllFeature("/home/zg/1T/samples/0", 2);

//    //------------------Detect (by change parameter tag)
//    //testDetect("/home/zg/1T/videos/download/1502350723-ee945449-8fea-4654-9868-eab636b307b8.nsf","TruckDetect");

//    //------------------Detect and Classfiy
//    //testDetectAndClassify("/home/zg/1T/videos/download/1502344105-9e6fa29d-cadd-4590-80b7-2609d1606ca4.nsf","CarDetect","CarShape",true,"CarColor",false);

//    //2017/8/13 new add
////    CarTracker testCarTrack;
////    //string carTrack(string videoFileName, string shape, string color, string logo="", string plate="");
////    string videoPath = "/home/zg/1T/videos/视频2/贡井区S207道树德门口-北向南卡口-车道1/510303000200000762-00010001-20170606070000-20170606080000-510303-01.nsf";
////   testCarTrack.carTrack(videoPath,"suv","white");

//    cv::Mat mat = cv::imread("/home/zg/115.jpg");
//    cvmat_to_avframe(mat);

//    return a.exec();
//}
