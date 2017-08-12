#include "CarDetect.hpp"

int main() {
    const string& mean_file = FLAGS_mean_file;
    const string& mean_value = FLAGS_mean_value;
    const float confidence_threshold = 0.5;
    Detector detector("/home/zg/traffic/QtProject/CarTracking/dependence/TruckDetect/deploy.prototxt", "/home/zg/traffic/QtProject/CarTracking/dependence/TruckDetect/model_iter_35219.caffemodel", mean_file, mean_value);

    cv::VideoCapture capture("/home/zg/1T/video/1502351242-1188b3ca-e82f-4c3d-a520-30c5469b6290.nsf");
    if (!capture.isOpened())
    {
        std::cout<<"fail to open"<<std::endl;
        return 0;
    }


    std::cout<<"frameCount:"<<capture.get(CV_CAP_PROP_FRAME_COUNT);
    int fps=capture.get(CV_CAP_PROP_FPS);
    cv::Mat frame;
    while (capture.read(frame))
    {
        std::vector<vector<float> > detections = detector.Detect(frame);

        for (int i = 0; i < detections.size(); ++i) {
            const vector<float>& d = detections[i];
            if (d[0] >= confidence_threshold) {
                std::cout << "has" <<std::endl;
                cv::Rect rect(d[1], d[2], (d[3] - d[1]), (d[4] - d[2]));
                cv::rectangle(frame, rect, cvScalar(0, 255, 0));
            }
        }

        cv::imshow("result", frame);
        cv::waitKey(1);
    }
    capture.release();
    return 0;
}
//int main() {
//    const string& mean_file = FLAGS_mean_file;
//    const string& mean_value = FLAGS_mean_value;
//    const float confidence_threshold = 0.25;
//    Detector detector("/home/zg/1T/car_detection/files/model/SSD_300x300/deploy.prototxt", "/home/zg/1T/car_detection/files/snapshot/SSD_300x300/VGG_VOC0712_SSD_300x300_iter_40587.caffemodel", mean_file, mean_value);

//    cv::VideoCapture capture("/home/zg/1T/test.nsf");
//    if (!capture.isOpened())
//    {
//        std::cout<<"fail to open"<<std::endl;
//        return 0;
//    }


//    std::cout<<"frameCount:"<<capture.get(CV_CAP_PROP_FRAME_COUNT);
//    int fps=capture.get(CV_CAP_PROP_FPS);
//    cv::Mat frame;
//    while (capture.read(frame))
//    {
//        Mat temp =frame(Rect(frame.cols/2-1,0,frame.cols/2-1,frame.rows/2-1));
//        std::vector<vector<float> > tdetections = detector.Detect(temp);

//        std::vector<vector<float> > detections = detector.Detect(frame);

//        for(std::vector<vector<float> >::iterator it=detections.begin();it!=detections.end();)
//        {
//            if(((*it)[1]>=(frame.cols/2-1))&&((*it)[2]<=(frame.rows/2-1)))
//            {
//                it=detections.erase(it);
//                continue;
//            }
//            it++;
//        }

//        for (int i = 0; i < detections.size(); ++i) {
//            const vector<float>& d = detections[i];
//            if (d[0] >= confidence_threshold) {
//                cv::Rect rect(d[1], d[2], (d[3] - d[1]), (d[4] - d[2]));
//                cv::rectangle(frame, rect, cvScalar(0, 255, 0));
//            }
//        }

//        for (int i = 0; i < tdetections.size(); ++i) {
//            const vector<float>& d = tdetections[i];
//            if (d[0] >= confidence_threshold) {
//                cv::Rect rect(d[1]+frame.cols/2-1, d[2], (d[3] - d[1]), (d[4] - d[2]));
//                cv::rectangle(frame, rect, cvScalar(0, 255, 0));
//            }
//        }

//        cv::imshow("result", frame);
//        cv::waitKey(1);
//    }
//    capture.release();
//    return 0;
//}
