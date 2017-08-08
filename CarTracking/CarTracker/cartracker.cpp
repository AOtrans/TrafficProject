#include "cartracker.h"

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
    if(tracker != nullptr)
    {
        delete tracker;
        tracker = nullptr;
    }
}

std::vector<Prediction> CarTracker::getLogo(const cv::Mat &img, int top_k)
{
    qDebug() << "logoExtract init";

    if(logoExtract == nullptr)
        logoExtract = new CarFeatureExtract("CarLogo");

    return logoExtract->singleImageCarFeatureExtract(img, top_k);
}

std::vector<Prediction> CarTracker::getShape(const cv::Mat &img, int top_k)
{
    qDebug() << "shapeExtract init";

    if(shapeExtract == nullptr)
        shapeExtract = new CarFeatureExtract("CarShape");

    return shapeExtract->singleImageCarFeatureExtract(img, top_k);
}

std::vector<Prediction> CarTracker::getColor(const cv::Mat &img, int top_k)
{
    qDebug() << "colorExtract init";

    if(colorExtract == nullptr)
        colorExtract = new CarFeatureExtract("CarColor");

    return colorExtract->singleImageCarFeatureExtract(img, top_k);
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

float *CarTracker::getSiftFeature(Mat &img)
{
    SiftDescriptorExtractor extractor;
        vector<KeyPoint> keypoints;
        cv::Mat des;
        float *data=new float[128];
        float *temp=data;
        int step = 10; // 10 pixels spacing between kp's

        for (int y=step; y<src.rows-step; y+=step){
            for (int x=step; x<src.cols-step; x+=step){
                // x,y,radius
                keypoints.push_back(KeyPoint(float(x), float(y), float(step)));
            }
        }
        extractor.compute(img, keypoints, des);

        for(int i=0;i<des.cols;i++)
        {
            float max=0.0;
            for(int j=0;j<des.rows;j++)
            {
                max=fabs(pcaDes.at<float>(j,i))>max?fabs(pcaDes.at<float>(j,i)):max;
            }
            *(temp++)=max;
        }

        return data;
}

string CarTracker::carTrack(string videoFileName, string shape, string color, string logo, string plate)
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
        vector<cv::Rect> &&cars = getCars(frame);

        for(vector<cv::Rect>::iterator it = cars.begin(); it != cars.end(); it++)
        {
            auto dcolor = colorExtract->singleImageCarFeatureExtract(frame(*it).clone());
            auto dshape = shapeExtract->singleImageCarFeatureExtract(frame(*it).clone());

            if(compareColor(dcolor, color)
                    &&compareShape(dshape, shape))
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

    if (!capture.isOpened())
    {
        qDebug() << "capture not opened";
        return;
    }

    float rate = capture.get(CV_CAP_PROP_FPS);
    float usPerFrame = 1000/rate;

    cv::Mat frame;
    QDateTime time = QDateTime::fromTime_t(atoi(startTime.c_str()));
    int cross = 100;
    int num = 0;

    while (capture.read(frame))
    {
        if((num++)%cross != 0)
        {
            time.addMSecs(usPerFrame);
            continue;
        }

        getTrucks(frame, QString::number(time.toTime_t()), QString(channelCode.c_str()));
    }

    capture.release();
}

void CarTracker::taxiTrack(string videoFileName, string startTime, string channelCode)
{
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
    int cross = 100;
    int num = 0;

    while (capture.read(frame))
    {
        if((num++)%cross != 0)
        {
            time.addMSecs(usPerFrame);
            continue;
        }

        vector<cv::Rect> &&cars = getCars(frame);

        for(vector<cv::Rect>::iterator it = cars.begin(); it != cars.end(); it++)
        {
            auto dcolor = colorExtract->singleImageCarFeatureExtract(frame(*it).clone());
            auto dshape = shapeExtract->singleImageCarFeatureExtract(frame(*it).clone());

            if(dcolor.at(0).first == "green" && dshape.at(0).first == "car")
            {
                QString imageUrl = imageSavePath + "/" + QUuid::createUuid().toString() + ".jpg";
                cv::imwrite(imageUrl.toStdString(),frame(*it));
                QString sql = "insert into carPic_info(channel_id,carPic_time,carPic_url,carPic_type) values('%1','%2','%3','%4')";

                dbManager.execQuery(sql.arg(QString(channelCode.c_str()) ).arg(QString::number(time.toTime_t()) ).arg(imageUrl).arg("taxi"));
            }
        }
    }

    capture.release();
}

void CarTracker::areaCarTrack(string videoFileName, string startTime, string channelCode, string areas)
{
    QStringList t = QString(areas.c_str()).split(",");
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
        if((num++)%cross != 0)
        {
            time.addMSecs(usPerFrame);
            continue;
        }

        vector<cv::Rect> &&cars = getCars(frame);

        for(vector<cv::Rect>::iterator it = cars.begin(); it != cars.end(); it++)
        {
            if(inRect(*it,rectAreas, frame.cols, frame.rows))
            {
                QString imageUrl = imageSavePath + "/" + QUuid::createUuid().toString() + ".jpg";
                cv::imwrite(imageUrl.toStdString(),frame(*it));
                QString sql = "insert into carPic_info(channel_id,carPic_time,carPic_url,carPic_type) values('%1','%2','%3','%4')";

                dbManager.execQuery(sql.arg(QString(channelCode.c_str()) ).arg(QString::number(time.toTime_t()) ).arg(imageUrl).arg("areacar"));
            }
        }
    }

    capture.release();
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
        confidenceThreshold = util.getValue(tagName, "ConfidenceThreshold").toFloat();

        carDetector = new Detector(model_file, trained_file, mean_file, mean_value);
    }

    int pad=10;
    std::vector<vector<float> > detections = carDetector->Detect(img);

    for (int i = 0; i < detections.size(); ++i) {
        const vector<float> &d = detections[i];
        if (d[0] >= confidenceThreshold) {

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

void CarTracker::getTrucks(Mat &img, QString time, QString channelCode)
{
    if(truckDetector == nullptr)
    {
        qDebug() << "truck detector init";

        const char* tagName = "TruckDetect";
        string model_file   = util.getValue(tagName, "modelFilePath").toStdString();
        string trained_file = util.getValue(tagName, "trainedFilePath").toStdString();
        string mean_file    = util.getValue(tagName, "meanFilePath").toStdString();
        string mean_value   = util.getValue(tagName, "meanValue").toStdString();
        confidenceThreshold = util.getValue(tagName, "ConfidenceThreshold").toFloat();

        truckDetector = new Detector(model_file, trained_file, mean_file, mean_value);
    }

    int pad=10;
    std::vector<vector<float> > detections = truckDetector->Detect(img);

    for (int i = 0; i < detections.size(); ++i) {
        const vector<float>& d = detections[i];
        if (d[0] >= confidenceThreshold) {

            int x=(d[1]-pad > 0)?(d[1]-pad):0;
            int y=(d[2]-pad > 0)?(d[2]-pad):0;
            int width=(d[3]+pad < img.cols)?(d[3]-d[1]+pad):(img.cols-d[1]);
            int height=(d[4]+pad < img.rows)?(d[4]-d[2]+pad):(img.rows-d[2]);

            cv::Rect rect(x, y, width, height);
            QString imageUrl = imageSavePath + "/" + QUuid::createUuid().toString() + ".jpg";
            cv::imwrite(imageUrl.toStdString(), img(rect));
            QString sql = "insert into carPic_info(channel_id,carPic_time,carPic_url,carPic_type) values('%1',%2,'%3','%4')";

            dbManager.execQuery(sql.arg(channelCode).arg(time).arg(imageUrl).arg("truck"));
        }
    }
}

}

bool CarTracker::compareCOSLike(float *t1, float *t2, int count)
{
    float dotSum=0.0f;
    float modeT1=0.0f,modeT2=0.0f;
    for(int i=0;i<count;i++)
    {
        dotSum+=t1[i]*t2[i];
        modeT1+=t1[i]*t1[i];
        modeT2+=t2[i]*t2[i];
    }
    if(modeT1==0||modeT2==0)
        return false;
    else
        return (acos(dotSum/(sqrt(fabs(modeT1))*sqrt(fabs(modeT2))))<=0.1);
}

bool CarTracker::compareShape(std::vector<Prediction> &result,string shape)
{
    for(int i=0;i<2;i++)
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
