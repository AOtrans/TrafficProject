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

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ::google::InitGoogleLogging("");
    //CarShape Classify (by change parameter tag)
    //testClassify("/home/zg/1T/samples/0","CarShape",true);

    //CarColor Classify (by change parameter tag)
    //testClassify("/home/zg/1T/samples/0","CarColor");

    //MotoShape Classify vaild
    //testClassify("/home/zg/1T/test","MotoShape");

    //CarShape and CarColor Classify
    //testCarAllFeature("/home/zg/1T/samples/0", 2);

    //    Detect (by change parameter tag)
//        testDetect("/home/zg/1T/videos/download/1502350723-ee945449-8fea-4654-9868-eab636b307b8.nsf","TruckDetect");

    //Detect and Classfiy
    //    testDetectAndClassify("/home/zg/1T/videos/download/1502344105-9e6fa29d-cadd-4590-80b7-2609d1606ca4.nsf","CarDetect","CarShape",true,"CarColor",false);

    //taxiTrack("/home/zg/1T/videos/download/1502261462-189e4556-b4b5-45f2-aa23-15d62ed9f8d9.nsf", "1502586938", "0");
    truckTrack("/home/zg/1T/videos/download/1502350723-ee945449-8fea-4654-9868-eab636b307b8.nsf", "1502586938", "0");

//    cv::Mat m1 = cv::imread("/home/zg/1T/test2/5.jpg");
//    cv::Mat m2 = cv::imread("/home/zg/1T/test2/a.jpg");

//    std::cout<<compareCOSLike(getSiftFeature(m1),getSiftFeature(m2),128)<<std::endl;

    return a.exec();
}
