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
            }
        }

        cv::imshow("result", frame);
        cv::waitKey(1);
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

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    //CarShape Classify (by change parameter tag)
    //testClassify("/home/zg/1T/samples/0","CarShape",true);

    //CarColor Classify (by change parameter tag)
    //testClassify("/home/zg/1T/samples/0","CarColor");

    //MotoShape Classify vaild
    //testClassify("/home/zg/1T/test","MotoShape");

    //CarShape and CarColor Classify
    //testCarAllFeature("/home/zg/1T/samples/0", 2);

    //Detect (by change parameter tag)
    //testDetect("/home/zg/1T/videos/download/1502350723-ee945449-8fea-4654-9868-eab636b307b8.nsf","TruckDetect");

    //Detect and Classfiy
    testDetectAndClassify("/home/zg/1T/videos/download/1502344105-9e6fa29d-cadd-4590-80b7-2609d1606ca4.nsf","CarDetect","CarShape",true,"CarColor",false);

    return a.exec();
}
