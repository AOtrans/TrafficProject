#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

#ifndef BASICOPERATION
#define BASICOPERATION

class basicOperation
{
public:
	int myOSTU(Mat &mat);                                 //OSTU得到二值化阈值
	void verticalProjection(Mat &image,vector<int> &pixSum);   //垂直投影
	void horizontalProjection(Mat &image,vector<int> &pixSum); //水平投影
	Mat getColorArea(Mat src,Mat &mask);                       //得到颜色区域

	Mat getColorMaskBlue(Mat src);                                          //得到蓝色区域
	Mat getColorMaskYellow(Mat src);                                        //得到黄色区域
	int plateType(Mat src);                            //判断车牌类型,返回值1 蓝色车牌  返回值2 黄色车牌
	void getGrayFromYellow(Mat &image);                    //黄牌车转换为同一格式 
};

#endif
