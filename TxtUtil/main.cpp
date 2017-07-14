#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <iostream>
using namespace cv;
void doit(Mat src)
{
    //gamma校正
    src.convertTo(src,CV_32FC1);
    cv::pow(src,1.0/2.2,src);
    cv::normalize(src,src,0,255,cv::NORM_MINMAX,CV_8UC1);
    cv::namedWindow("gamma",0);
    float temp=((float)src.rows)/((float)src.cols);
    cv::Mat dst(256*temp,256,src.type());
    resize(src,dst,dst.size());

            SIFT detector(100);
            std::vector<KeyPoint> keypoints;
            detector.detect( dst, keypoints );
            Mat show;
            drawKeypoints(dst,keypoints,show);
                cv::imshow("gamma",show);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
//    cv::Mat src=cv::imread("D:/843.jpeg",CV_LOAD_IMAGE_GRAYSCALE);
//    cv::Mat src2=cv::imread("D:/199.jpeg",CV_LOAD_IMAGE_GRAYSCALE);
//   doit(src);
   //doit(src2);

    return a.exec();
}
