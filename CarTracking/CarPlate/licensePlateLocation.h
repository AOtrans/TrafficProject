#include <opencv2/opencv.hpp>
#include <iostream>
#include "basicOperation.h"
#include "isPate.h"

using namespace std;
using namespace cv;

/*
*车牌定位类
*通过边缘检测和连通域
*/

class licensePlateLocation                                             //车牌定位类
{
private:
	basicOperation bso;
	isPate getPlate;
	string plateSvmTrainModel;
private:	
	void horizontalConnection(Mat &image);                             //水平连接
	void isLicensePlate(vector<vector<int>> &ableRectangle,Mat &image);                 //判断区域是否为车牌

public:
	void setPlateSvmTrainModel(string plateSvmTrainModel);
	void setIsPateParam();
	void process(Mat &image,vector<Mat> &plate);

};
