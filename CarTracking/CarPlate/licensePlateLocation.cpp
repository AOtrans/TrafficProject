#include "licensePlateLocation.h"

#include "colorImageEdgeDetect.h"
#include "getConnectedDomain.h"

void licensePlateLocation::setPlateSvmTrainModel(string plateSvmTrainModel)
{
	this->plateSvmTrainModel = plateSvmTrainModel;
}

void licensePlateLocation::setIsPateParam()
{
	getPlate.setTrainModelPath(plateSvmTrainModel);
}

void licensePlateLocation::horizontalConnection(Mat &image)                             //水平连接
{
	int i,j,jj;
	int rows = image.rows;
	int cols = image.cols;
	int left = 0,right = 0;

	for(i = 0;i < rows;i++)
	{
		uchar *ptr = image.ptr<uchar>(i);
		for(j = 0;j < cols;j++)
		{
			if(ptr[j] != 0)
			{
				left = j;
				while(j != cols-1 && ptr[++j] == 0);
				right = j--;

				if(right - left < 20)       //可调整参数，隐患?????????????????????????????????????????????????????????????????????????????????
					for(jj = left;jj < right;jj++)
						ptr[jj] = 255;
			}
		}
	}
}

void licensePlateLocation::isLicensePlate(vector<vector<int>> &ableRectangle,Mat &image)                 //判断区域是否为车牌
{
	//根据svm算法筛选车牌区域
	int i,j;
	int ableRectangleLength = ableRectangle.size();
	vector<vector<int>> temp;
	int area;
	float rate;
	int height,length;

	for(i = 0;i < ableRectangleLength;i++)
	{
		//先通过面积筛选掉面积过小的区域
		height = ableRectangle[i][1] - ableRectangle[i][0];
		length = ableRectangle[i][3] - ableRectangle[i][2];
		rate = length*1.0 / height;
		area = length * height;
		if(area < 1000 || area > 12000)
			continue;
		if(rate < 2.0 || rate > 7.0)
			continue;

		//通过svm筛选
		Mat pate = image(Rect(Point(ableRectangle[i][2],ableRectangle[i][0]),Point(ableRectangle[i][3],ableRectangle[i][1])));

		//通过车牌二值图像像素跳变的次数筛选车牌
		Mat testImage = pate.clone();
		int T = bso.myOSTU(testImage);
		threshold(testImage,testImage,T,255,THRESH_BINARY);
		int row = testImage.rows / 2;
		int cols = testImage.cols;
		uchar* ptr = testImage.ptr<uchar>(row);
		uchar thePrePix = ptr[0];
		int changeSum = 0;
		for(j = 1;j < cols;j++)
		{
			if(thePrePix != ptr[j])
				changeSum++;
			thePrePix = ptr[j];
		}
		if(changeSum < 13)
			continue;

		int type = bso.plateType(pate);
		Mat imageGray;
		if(type == 1)                 //蓝牌车
		{
			Mat myMask;
			myMask = bso.getColorMaskBlue(pate);
			int arry[3][3] = {1,1,1,1,1,1,1,1,1};
			Mat myKernel(3,3,CV_8UC1,arry);
			morphologyEx(myMask,myMask,MORPH_CLOSE,myKernel,Point(-1,-1),1);
			morphologyEx(myMask,myMask,MORPH_OPEN,myKernel,Point(-1,-1),1);
			Mat isPlate = bso.getColorArea(pate,myMask);

			int testImageRows = isPlate.rows;
			if(testImageRows < 2)
				isPlate = pate.clone();

			cvtColor(isPlate,imageGray,CV_BGR2GRAY);
		}
		else if(type == 2)               //黄牌车
		{
			Mat myMask;
			myMask = bso.getColorMaskYellow(pate);
			int arry[3][3] = {1,1,1,1,1,1,1,1,1};
			Mat myKernel(3,3,CV_8UC1,arry);
			morphologyEx(myMask,myMask,MORPH_CLOSE,myKernel,Point(-1,-1),1);
			morphologyEx(myMask,myMask,MORPH_OPEN,myKernel,Point(-1,-1),1);

			Mat isPlate = bso.getColorArea(pate,myMask);

			int testImageRows = isPlate.rows;
			if(testImageRows < 2)
				isPlate = pate.clone();

			cvtColor(isPlate,imageGray,CV_BGR2GRAY);
			bso.getGrayFromYellow(imageGray);
		}

		int svmResult = (int)getPlate.svmPredict(imageGray);
		if(svmResult == 1)
		{
			temp.push_back(ableRectangle[i]);
		}
	}

	ableRectangle.clear();
	int tempLength = temp.size();
	for(i = 0;i < tempLength;i++)
		ableRectangle.push_back(temp[i]);
}

void licensePlateLocation::process(Mat &image,vector<Mat> &plate)
{
	colorImageEdgeDetect edge;

	Mat outputImage = edge.process(image);

	horizontalConnection(outputImage);

	int arry[3][3] = {1,1,1,1,1,1,1,1,1};
	Mat myKernel(3,3,CV_8UC1,arry);
	morphologyEx(outputImage,outputImage,MORPH_OPEN,myKernel,Point(-1,-1),2);

	vector<vector<int>> ableRectangle;

	getConnectedDomain domain;
	domain.findDomainBaseStrokeBaseOnRonny(outputImage,ableRectangle);

	isLicensePlate(ableRectangle,image);

	Mat imageShow = image.clone();
	int ableRectangleLength = ableRectangle.size();
	for(int i = 0;i < ableRectangleLength;i++)
	{
		Mat pushPate;
		pushPate = image.rowRange(ableRectangle[i][0],ableRectangle[i][1]);                          //车牌区域边界，加上容错值
		pushPate = pushPate.colRange(ableRectangle[i][2],ableRectangle[i][3]);
		plate.push_back(pushPate);

		rectangle(imageShow,Point(ableRectangle[i][2],ableRectangle[i][0]),Point(ableRectangle[i][3],ableRectangle[i][1]),Scalar(0,0,255),3);          //在图片中框处车牌
	}

	//imshow("车牌",imageShow);
	//waitKey();
}
