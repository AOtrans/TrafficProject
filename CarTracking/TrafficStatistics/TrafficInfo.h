#ifndef TRAFFIC_INFO_H
#define TRAFFIC_INFO_H
#include "CarDetect/cardetector.h"

class Trainfo{
public:
    Trainfo();
    void excute(Detector *detector, const string& capture_name, const string& outFile);
    void CrossingDemo(Detector *detector, const string& capture_name, const string& outFile, vector<vector<float> >rectboxes);
private:

    int mode;
    std::vector<vector<float> > box_transform(std::vector<vector<float> > box);
    float congestion(std::vector<vector<float> > results, cv::Mat img);
    int flow(std::vector<vector<float> > results, std::vector<vector<float> > &List, cv::Mat img);
    int car_count(std::vector<float> a, std::vector<vector<float> > b);
    float max_f(float a, float b);
    float min_f(float a, float b);
    float overlap(std::vector<float> a, std::vector<float> b);
    float singleoverlap(std::vector<float> a, std::vector<float> b);
};

#endif // TRAFFIC_INFO_H
