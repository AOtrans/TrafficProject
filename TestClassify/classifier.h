#ifndef CLASSIFIER_H
#define CLASSIFIER_H
#include "common.h"

using namespace caffe;  // NOLINT(build/namespaces)
using namespace std;

/* Pair (label, confidence) representing a prediction. */
typedef std::pair<string, float> Prediction;

class Classifier {
public:
    Classifier(const string& model_file,
               const string& trained_file,
               const string& mean_file,
               const string& mean_value,
               const string& label_file,
               bool ifScale);

    std::vector<Prediction> Classify(const cv::Mat &img, int N = 5);

private:
    void SetMean(const string& mean_file, const string &mean_value);

    std::vector<float> Predict(const cv::Mat &img);

    void WrapInputLayer(std::vector<cv::Mat> *input_channels);

    void Preprocess(const cv::Mat& img, std::vector<cv::Mat>* input_channels);

private:
    boost::shared_ptr<Net<float> > net_;
    cv::Size input_geometry_;
    int num_channels_;
    cv::Mat mean_;
    std::vector<string> labels_;
    bool ifScale;
};

#endif // CLASSIFIER_H
