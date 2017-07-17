#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

class colorImageEdgeDetect                           //彩色边缘检测类
{
private:
	Mat RGB2HSI(Mat &image);                         //颜色空间变换
	uchar myMin(uchar a,uchar b,uchar c);
	Mat HedgeV(Mat &Himage,Mat &Simage);              //H分量边缘检测（垂直边缘检测）
	Mat SIedgeV(Mat &image);                         //S I 分量边缘检测（垂直边缘检测）
	Mat HedgeH(Mat &Himage,Mat &Simage);              //H分量边缘检测（水平边缘检测）
	Mat SIedgeH(Mat &image);                         //S I 分量边缘检测（水平边缘检测）
	float differ(float a1,float a2,float a3,float a4);

	Mat localIntensity(Mat &V,Mat &H);                                 //局部边缘强度，综合垂直，水平边缘检测
	Mat colorIntensity(Mat &H,Mat &S,Mat &I,float p);                  //彩色边缘强度

	float theMaxValue(Mat &mat);
	void normalized(Mat &image);                                       //归一化
	Mat To255(Mat &Image);

	Mat edgeV(Mat &image);                            //垂直边缘
	Mat edgeVH(Mat &image);                           //垂直 水平边缘

public:
	Mat process(Mat &image);
};
