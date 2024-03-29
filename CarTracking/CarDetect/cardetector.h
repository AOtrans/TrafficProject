#ifndef CARDETECTOR_H
#define CARDETECTOR_H
#include "common.h"

class Detector {
public:
    Detector(const string& model_file,
        const string& weights_file,
        const string& mean_file,
        const string& mean_value);

    std::vector<vector<float> > Detect(const cv::Mat& img);

private:
    void SetMean(const string& mean_file, const string& mean_value);

    void WrapInputLayer(std::vector<cv::Mat>* input_channels);

    void Preprocess(const cv::Mat& img,
        std::vector<cv::Mat>* input_channels);

private:
    boost::shared_ptr<Net<float> > net_;
    cv::Size input_geometry_;
    int num_channels_;
    cv::Mat mean_;
};

#endif // CARDETECTOR_H
