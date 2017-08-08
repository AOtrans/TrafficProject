#include "CarDetect.hpp"

//int main() {
//    const string& mean_file = FLAGS_mean_file;
//    const string& mean_value = FLAGS_mean_value;
//    const float confidence_threshold = 0.25;
//    Detector detector("/home/zg/1T/car_detection/files/model/SSD_300x300/deploy.prototxt", "/home/zg/1T/car_detection/files/snapshot/SSD_300x300/VGG_VOC0712_SSD_300x300_iter_40587.caffemodel", mean_file, mean_value);

//    cv::VideoCapture capture("/home/zg/1T/video/视频2/贡井区卷尺厂三叉路口-北向南全景/510303000200000384-00010001-20170601080000-20170601090000-510303-01.nsf");
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
//        std::vector<vector<float> > detections = detector.Detect(frame);
//        for (int i = 0; i < detections.size(); ++i) {
//            const vector<float>& d = detections[i];
//            if (d[0] >= confidence_threshold) {
//                cv::Rect rect(d[1], d[2], (d[3] - d[1]), (d[4] - d[2]));
//                cv::rectangle(frame, rect, cvScalar(0, 255, 0));
//            }
//        }

//        cv::imshow("result", frame);
//        cv::waitKey(1);
//    }
//    capture.release();
//    return 0;
//}
int main() {
    const string& mean_file = FLAGS_mean_file;
    const string& mean_value = FLAGS_mean_value;
    const float confidence_threshold = 0.25;
    Detector detector("/home/zg/1T/car_detection/files/model/SSD_300x300/deploy.prototxt", "/home/zg/1T/car_detection/files/snapshot/SSD_300x300/VGG_VOC0712_SSD_300x300_iter_40587.caffemodel", mean_file, mean_value);

    cv::VideoCapture capture("/home/zg/1T/video/视频2/贡井区卷尺厂三叉路口-北向南全景/510303000200000384-00010001-20170601080000-20170601090000-510303-01.nsf");
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
        Mat temp =frame(Rect(frame.cols/2-1,0,frame.cols/2-1,frame.rows/2-1));
        std::vector<vector<float> > tdetections = detector.Detect(temp);

        std::vector<vector<float> > detections = detector.Detect(frame);

        for(std::vector<vector<float> >::iterator it=detections.begin();it!=detections.end();)
        {
            if(((*it)[1]>=(frame.cols/2-1))&&((*it)[2]<=(frame.rows/2-1)))
            {
                it=detections.erase(it);
                continue;
            }
            it++;
        }

        for (int i = 0; i < detections.size(); ++i) {
            const vector<float>& d = detections[i];
            if (d[0] >= confidence_threshold) {
                cv::Rect rect(d[1], d[2], (d[3] - d[1]), (d[4] - d[2]));
                cv::rectangle(frame, rect, cvScalar(0, 255, 0));
            }
        }

        for (int i = 0; i < tdetections.size(); ++i) {
            const vector<float>& d = tdetections[i];
            if (d[0] >= confidence_threshold) {
                cv::Rect rect(d[1]+frame.cols/2-1, d[2], (d[3] - d[1]), (d[4] - d[2]));
                cv::rectangle(frame, rect, cvScalar(0, 255, 0));
            }
        }

        cv::imshow("result", frame);
        cv::waitKey(1);
    }
    capture.release();
    return 0;
}
