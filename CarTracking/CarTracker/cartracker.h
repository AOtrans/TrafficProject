#ifndef CARTRACKER_H
#define CARTRACKER_H
#include "common.h"
#include "CarFeature/carfeatureextract.h"

class CarTracker
{
public:
    CarTracker* getInstence(const char* configFilePath="");
    ~CarTracker();
    std::vector<Prediction> getLogo(const cv::Mat &img,int top_k);
    std::vector<Prediction> getShape(const cv::Mat &img,int top_k);
    std::vector<Prediction> getColor(const cv::Mat &img,int top_k);
    string getPlate(const cv::Mat &img);
    string carTrack(string videoFileName,string shape,string color,string logo="",string plate="");
private:
    vector<cv::Rect> getCars(Mat &img);
    bool compareShape(std::vector<Prediction> &result,string shape);
    bool compareColor(std::vector<Prediction> &result,string shape);
    CarFeatureExtract *shapeExtract,*colorExtract,*logoExtract;
    const char* configFilePath;
    CarTracker(const char* configFilePath);
    static CarTracker* tracker;
};

#endif // CARTRACKER_H
