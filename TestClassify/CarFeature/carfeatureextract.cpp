#include "carfeatureextract.h"

CarFeatureExtract::CarFeatureExtract(QString tagName):classifier(NULL)
{
    if(tagName != "")
    {
        string model_file   = util.getValue(tagName, "modelFilePath").toStdString();
        string trained_file = util.getValue(tagName, "trainedFilePath").toStdString();
        string mean_file    = util.getValue(tagName, "meanFilePath").toStdString();
        string mean_value   = util.getValue(tagName, "meanValue").toStdString();
        string label_file   = util.getValue(tagName, "labelFilePath").toStdString();
        bool ifScale = (util.getValue(tagName, "ifScale").toStdString() == "true");

        classifier = new Classifier(model_file, trained_file, mean_file, mean_value, label_file, ifScale);
    }
}

CarFeatureExtract::CarFeatureExtract():classifier(NULL)
{
    qDebug() << "create Extractor with no init";
}

CarFeatureExtract::~CarFeatureExtract()
{
    if(classifier != NULL)
        delete classifier;
}

bool CarFeatureExtract::reInit(QString tagName)
{
    if(classifier != NULL)
        delete classifier;

    if(tagName != "")
    {
        string model_file   = util.getValue(tagName, "modelFilePath").toStdString();
        string trained_file = util.getValue(tagName, "trainedFilePath").toStdString();
        string mean_file    = util.getValue(tagName, "meanFilePath").toStdString();
        string mean_value   = util.getValue(tagName, "meanValue").toStdString();
        string label_file   = util.getValue(tagName, "labelFilePath").toStdString();
        bool ifScale = (util.getValue(tagName, "ifScale").toStdString() == "true");

        classifier = new Classifier(model_file, trained_file, mean_file, mean_value, label_file, ifScale);
        return true;
    }
    return false;
}
/*
bool CarFeatureDicter::checkImageFormat(cv::Mat &img)
{
    if(img.empty())
    {
        std::cout<<"image is empty"<<std::endl;
        return false;
    }
    if(img.channels()!=3)
    {
        std::cout<<"current channel:"<<img.channels()<<" resize to standard channel 3"<<std::endl;
        cv::cvtColor(img,img,CV_GRAY2BGR);
    }
    if(img.rows!=256||img.cols!=256)
    {
        std::cout<<"resize to standard scale 256*256"<<std::endl;
        cv::resize(img,img,cv::Size(256,256));
    }
    return true;
}
*/
std::vector<Prediction> CarFeatureExtract::singleImageCarFeatureExtract(cv::Mat img, int top_k) {
    if(classifier != NULL)
    {
        qDebug() << "---------- Prediction for " << "inputImage" << " ----------";

        return classifier->Classify(img, top_k);
    }
    else
    {
        qDebug() << "please reInit classifier first";

        return std::vector<Prediction>();
    }
}

std::vector<Prediction> CarFeatureExtract::singleImagePathCarFeatureExtract(const char *imageFilePath, int top_k) {
    if(classifier != NULL)
    {
        cv::Mat img = cv::imread(imageFilePath);
        qDebug() << "---------- Prediction for " << imageFilePath << " ----------";

        return classifier->Classify(img, top_k);
    }
    else
    {
        qDebug() << "please reInit classifier first";

        return std::vector<Prediction>();
    }
}

vector<std::vector<Prediction> > CarFeatureExtract::imagePathsCarFeatureExtract(const vector<const char*> &imageFilePaths, int top_k) {
    vector<std::vector<Prediction> > res;

    if(classifier != NULL)
    {
        for(vector<const char*>::const_iterator it = imageFilePaths.cbegin(); it != imageFilePaths.cend(); it++)
        {
            qDebug() << "---------- Prediction for " << *it << " ----------";

            string imageFilePath = string(*it);
            cv::Mat img = cv::imread(imageFilePath);

            res.push_back(classifier->Classify(img, top_k));
        }
    }
    else
    {
        qDebug() << "please reInit classifier first";
    }

    return res;
}

vector<std::vector<Prediction> > CarFeatureExtract::imagesCarFeatureExtract(const vector<cv::Mat> &images, int top_k) {
    vector<std::vector<Prediction> > res;

    if(classifier != NULL)
    {
        int num=0;
        for(vector<cv::Mat>::const_iterator it = images.cbegin(); it != images.cend(); it++)
        {
            qDebug() << "---------- Prediction for " << "inputImage " << num << " ----------";

            cv::Mat img=*it;
            res.push_back(classifier->Classify(img, top_k));
            num++;
        }
    }
    else
    {
        qDebug() << "please reInit classifier first";
    }

    return res;
}
