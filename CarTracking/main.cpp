#include "CarTracker/cartracker.h"

int main(int argc, char *argv[])
{
    ::google::InitGoogleLogging(argv[0]);
    CarTracker *c=CarTracker::getInstence("/home/zg/traffic/QtProject/CarTracking/config.ini");
    Mat img =imread("/home/zg/traffic/temp/plate.jpg");
    std::cout<<c->getPlate(img)<<std::endl;\
    imshow("dd",img);
//    //cout<<c->carTrack("D:/work/ppt/xx.mp4","car","white","","")<<endl;
//    Mat img =imread("/home/zg/traffic/temp/test2.jpg");
//    vector<Rect> &&cars=c->getCars(img);
//    for(vector<Rect>::iterator it=cars.begin();it!=cars.end();it++)
//    {
//        cv::rectangle(img,*it,Scalar(255,0,0),2);
//    }
//    imshow("dd",img);
    cv::waitKey();
    return 0;
}
