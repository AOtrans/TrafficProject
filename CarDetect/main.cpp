#include "CarDetect.hpp"

int main() {
    const string& mean_file = FLAGS_mean_file;
    const string& mean_value = FLAGS_mean_value;
    const float confidence_threshold = 0.2;

    Detector detector("/home/zg/1T/caffe_models/QS_detect_models/ssd_deploy.prototxt", "/home/zg/1T/caffe_models/QS_detect_models/ssd.caffemodel", mean_file, mean_value);
    cv::Mat img = cv::imread("/home/zg/traffic/temp/1.png");
    std::vector<vector<float> > detections = detector.Detect(img);
    for (int i = 0; i < detections.size(); ++i) {
        const vector<float>& d = detections[i];
        if (d[0] >= confidence_threshold) {
            cv::Rect rect(d[1], d[2], (d[3] - d[1]), (d[4] - d[2]));
            cv::rectangle(img, rect, cvScalar(0, 255, 0), 1, 8, 0);
        }
    }
    cv::imshow("result", img);
    cv::waitKey(0);
    return 0;
}
