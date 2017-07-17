#include "carfeatureextract.h"

CarFeatureExtract::CarFeatureExtract(const char *configFilePath, const char *tagName):classifier(NULL)
{
    IniUtil util;
    if(util.OpenFile(configFilePath,"r")!=INI_SUCCESS)
    {
        std::cout<<"openConfigFileError"<<std::endl;
    }
    else
    {
        string model_file   = string(util.GetStr(tagName,"modelFilePath"));
        string trained_file = string(util.GetStr(tagName,"trainedFilePath"));
        string mean_file    = string(util.GetStr(tagName,"meanFilePath"));
        string mean_value    = string(util.GetStr(tagName,"meanValue"));
        string label_file   = string(util.GetStr(tagName,"labelFilePath"));;
        classifier=new Classifier(model_file, trained_file, mean_file, mean_value, label_file);
        util.CloseFile();
    }
}

CarFeatureExtract::CarFeatureExtract():classifier(NULL)
{
    std::cout<<"create dicker with no init"<<std::endl;
}

CarFeatureExtract::~CarFeatureExtract()
{
    if(classifier!=NULL)
        delete classifier;
}

bool CarFeatureExtract::reInit(const char *configFilePath, const char *TagName)
{
    IniUtil util;
    if(util.OpenFile(configFilePath,"r")!=INI_SUCCESS)
    {
        std::cout<<"openConfigFileError"<<std::endl;
        return false;
    }
    else
    {
        if(classifier!=NULL)
            delete classifier;
        string model_file   = string(util.GetStr(TagName,"modelFilePath"));
        string trained_file = string(util.GetStr(TagName,"trainedFilePath"));
        string mean_file    = string(util.GetStr(TagName,"meanFilePath"));
        string mean_value    = string(util.GetStr(TagName,"meanValue"));
        string label_file   = string(util.GetStr(TagName,"labelFilePath"));;
        classifier=new Classifier(model_file, trained_file, mean_file, mean_value, label_file);
        util.CloseFile();
        return true;
    }
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
    if(classifier!=NULL)
    {
        std::cout << "---------- Prediction for "
                  << "inputImage" << " ----------" << std::endl;
        return classifier->Classify(img,top_k);
    }
    else
    {
        std::cout<<"please reInit classifier first"<<std::endl;
        return std::vector<Prediction>();
    }
}

std::vector<Prediction> CarFeatureExtract::singleImagePathCarFeatureExtract(const char* imageFilePath, int top_k) {
    if(classifier!=NULL)
    {
        cv::Mat img=cv::imread(imageFilePath);
        std::cout << "---------- Prediction for "
                  << string(imageFilePath) << " ----------" << std::endl;
        return classifier->Classify(img,top_k);
    }
    else
    {
        std::cout<<"please reInit classifier first"<<std::endl;
        return std::vector<Prediction>();
    }
}

vector<std::vector<Prediction> > CarFeatureExtract::imagePathsCarFeatureExtract(const vector<const char*> &imageFilePaths, int top_k) {
    vector<std::vector<Prediction>> res;
    if(classifier!=NULL)
    {
        for(vector<const char*>::const_iterator it=imageFilePaths.cbegin();it!=imageFilePaths.cend();it++)
        {
            std::cout << "---------- Prediction for "
                      << string(*it) << " ----------" << std::endl;
            string imageFilePath=string(*it);
            cv::Mat img=cv::imread(imageFilePath);
            res.push_back(classifier->Classify(img,top_k));
        }
    }
    else
    {
        std::cout<<"please reInit classifier first"<<std::endl;
    }
    return res;
}

vector<std::vector<Prediction> > CarFeatureExtract::imagesCarFeatureExtract(const vector<cv::Mat> &images, int top_k) {
    vector<std::vector<Prediction>> res;
    if(classifier!=NULL)
    {
        int num=0;
        for(vector<cv::Mat>::const_iterator it=images.cbegin();it!=images.cend();it++)
        {
            std::cout << "---------- Prediction for "
                      << "inputImage " <<num<< " ----------" << std::endl;
            cv::Mat img=*it;
            res.push_back(classifier->Classify(img,top_k));
            num++;
        }
    }
    else
    {
        std::cout<<"please reInit classifier first"<<std::endl;
    }
    return res;
}
