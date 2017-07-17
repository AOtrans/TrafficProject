#ifndef CARTRACKER_H
#define CARTRACKER_H
#include "common.h"
#include "CarFeature/carfeatureextract.h"
#include "CarDetect/cardetector.h"
#include "CarPlate/Lprs.h"

class CarTracker
{
public:
    static CarTracker* getInstence(const char* configFilePath="");
    ~CarTracker();
    std::vector<Prediction> getLogo(const cv::Mat &img,int top_k);
    std::vector<Prediction> getShape(const cv::Mat &img,int top_k);
    std::vector<Prediction> getColor(const cv::Mat &img,int top_k);
    string getPlate(const cv::Mat &img);
    vector<cv::Rect> getCars(Mat &img);
    string carTrack(string videoFileName,string shape,string color,string logo="",string plate="");
private:
    bool compareShape(std::vector<Prediction> &result,string shape);
    bool compareColor(std::vector<Prediction> &result,string shape);
    CarTracker(const char* configFilePath);
    CarFeatureExtract *shapeExtract,*colorExtract,*logoExtract;
    Lprs *plateExtract;
    Detector* carDetector;
    const char* configFilePath;
    static CarTracker* tracker;
    float confidenceThreshold;
};

#endif // CARTRACKER_H
