#ifndef CARTRACKER_H
#define CARTRACKER_H

#include "common.h"
#include "CarFeature/carfeatureextract.h"
#include "CarDetect/cardetector.h"
#include "CarPlate/Lprs.h"
#include "TrafficStatistics/TrafficInfo.h"

class CarTracker
{
public:
    static CarTracker* getInstence();
    ~CarTracker();

    std::vector<Prediction> getLogo(const cv::Mat &img, int top_k);
    std::vector<Prediction> getShape(const cv::Mat &img, int top_k);
    std::vector<Prediction> getColor(const cv::Mat &img, int top_k);
    std::vector<Prediction> getMotoShape(const cv::Mat &img, int top_k);

    string getPlate(const cv::Mat &img);
    vector<cv::Rect> getCars(Mat &img);
    vector<cv::Rect> getMotos(Mat &img);
    void getTrucks(Mat &img,QString startTime, QString channelCode);

    string carTrack(string videoFileName, string shape, string color, string logo="", string plate="");

    string motoTrack(string videoFileName, string shape);

    void truckTrack(string videoFileName, string startTime, string channelCode);

    void taxiTrack(string videoFileName, string startTime, string channelCode);

    void areaCarTrack(string videoFileName, string startTime, string channelCode, string areas);

    string trafficStatistics(string videoFileName,string areas);

    pair<cv::Point2f,cv::Point2f> getPoints(float a1, float a2, float b1, float b2);
    bool inRect(cv::Rect rect, vector<pair<cv::Point2f, cv::Point2f> > rectAreas, int cols, int rows);

    QString getImageSavePath();

    bool compareCOSLike(float *t1, float *t2, int count);
    float getSiftFeature(Mat &img);
private:
    bool compareShape(std::vector<Prediction> &result, string shape);
    bool compareColor(std::vector<Prediction> &result, string shape);

    CarTracker();

    CarFeatureExtract *shapeExtract, *colorExtract, *logoExtract, *motoShapeExtract;
    Lprs *plateExtract;
    Trainfo *trainfo;
    Detector* carDetector, *truckDetector, *motoDetector;
    static CarTracker *tracker;
    float carThreshold, truckThreshold, motoThreshold;
    QString videoSavePath;
    QString imageSavePath;
};

#endif // CARTRACKER_H
