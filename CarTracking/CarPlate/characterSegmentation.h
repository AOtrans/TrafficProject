#include <opencv2/opencv.hpp>
#include <iostream>
#include "basicOperation.h"

using namespace cv;
using namespace std;

struct charPos
{
	int left;
	int right;
};

/*
*字符分割版本2
*投影的方法切割字符
*/

class characterSegmentation
{
private:
	basicOperation bso;
public:
	bool horizontalEdgeRemove(Mat &image,int &up,int &down);
	bool findZeroPosition(vector<int> &sumPix,int charHight,struct charPos charPosition[]);
	bool getRightPosition(struct charPos charPosition[],int charMaxLength,int charHight,int plateLength);     //当投影分割失败是转为 按比例分割
	void getTheChar(struct charPos charPosition[],int n,Mat &image,vector<Mat> &pateChar);

	//倾斜校正函数
	void getPoint(vector<Point> &point,Mat &image);
    bool getDegree(Mat &image,double &degree);
	Mat rotateImage(Mat img, double degree);             //旋转函数

	void plateVerticalCorrection(Mat &image,double &degree);       //垂直校正  获得倾斜角度
	int BlankPixSum(vector<int> &pixSum);
	Mat verticalCorrection(Mat &image,double degree);                 //从上往下看，绕点从左往右转 degree为正，从右往左degree为负


	void getStretchMinMax(Mat image,int &min,int &max);           //得到灰度拉升的最大值和最小值
	void grayscaleStretching(Mat &image);                   //灰度拉升

	Mat onlyRChannalStretchingBlue(Mat image);                  //仅对R 通道进行灰度拉伸    蓝牌
	Mat onlyRChannalStretchingYellow(Mat image);                //仅对R 通道进行灰度拉伸    黄牌

	double charDensity(Mat &image);
	void verticalEdgeRemove(Mat &image,int &left,int &right);
	int horizontalBlankSum(vector<int> horizontalPix);
	bool charChangeSum(Mat &image);
	int verticalHightSum(vector<int> verticalPix,int hight);

	bool process(Mat &Image,vector<Mat> &pateChar);
};
