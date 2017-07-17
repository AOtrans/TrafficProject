#include "basicOperation.h"

int basicOperation::myOSTU(Mat &mat)
{
	int grayLevel[257];                        //256放灰度总数
	float probability[256];                    //每级灰度概率
	memset(grayLevel,0,sizeof(grayLevel));
	memset(probability,0,sizeof(probability));
	int rows = mat.rows;
	int cols = mat.cols;
	int i,j;
	for(i = 0;i < rows;i++)
	{
		uchar *ptr = mat.ptr<uchar>(i);
		for(j = 0;j < cols;j++)
		{
			grayLevel[ptr[j]]++;              //统计每级灰度个数
			grayLevel[256] += ptr[j];         //灰度级总数
		}
	}
	int T = grayLevel[256] / (rows*cols);
	int TGrayLevel = 0;
	int total = 0;
	for(i = 0;i < T;i++)       //<T
	{
		TGrayLevel += grayLevel[i]*i;
		total += grayLevel[i];
	}
	int T1 = 0;
	if(total != 0)
		T1 = TGrayLevel / total;
	TGrayLevel = 0;
	total = 0;
	for(i = T;i < 256;i++)     //>=T
	{
		TGrayLevel += grayLevel[i]*i;
		total += grayLevel[i];
	}
	int T2 = 255;
	if(total != 0)
		T2 = TGrayLevel / total;
	float u = 0;                     //图像的总灰度均值
	float u0,u1;
	float uk = 0;
	float w0 = 0;
	for(i = 0;i < 256;i++)
	{
		probability[i] = grayLevel[i] / (rows*cols*1.0);
		u += i*probability[i];
	}
	float ostuT[2] = {0,-10};
	float variance = 0;
	for(i = T1;i <= T2;i++)                    //OSTU算法开始
	{
		uk = 0;
		w0 = 0;
		for(j = 0;j < i;j++)
		{
			uk += j*probability[j];
			w0 += probability[j];
		}
		u0 = uk / w0;
		u1 = (u - uk) / (1 - w0);
		variance = w0*(u - u0)*(u - u0) + (1 - w0)*(u - u1)*(u - u1);
		if(variance > ostuT[1])
		{
			ostuT[0] = i;
			ostuT[1] = variance;
		}
	}
	return (int)ostuT[0];
}
void basicOperation::verticalProjection(Mat &image,vector<int> &pixSum)
{
	int rows = image.rows;
	int cols = image.cols;
	int i,j;
	//vector<int>pixSum;
	for(i = 0;i < cols;i++)
	{
		pixSum.push_back(0);
		for(j = 0;j < rows;j++)
		{
			if(image.at<uchar>(j,i) != 0)
				pixSum[i]++;
		}
	}

	Mat vertical = Mat::zeros(rows,cols,CV_8UC1);             //显示垂直投影结果
	for(i = 0;i < cols;i++)
	{
		for(j = 0;j < pixSum[i];j++)
			vertical.at<uchar>(j,i) = 255;
	}
	//namedWindow("verticalProjection",0);
	//imshow("verticalProjection",vertical);
	//waitKey();
}

void basicOperation::horizontalProjection(Mat &image,vector<int> &pixSum)
{
	int rows = image.rows;
	int cols = image.cols;
	int i,j;
	//vector<int>pixSum;
	for(i = 0;i < rows;i++)
	{
		pixSum.push_back(0);
		uchar *ptr = image.ptr<uchar>(i);
		for(j = 0;j < cols;j++)
		{
			if(ptr[j] != 0)                    //隐患？？？？？？？？？？？？？区分目标与背景
				pixSum[i]++;
		}
	}

	Mat horizontal = Mat::zeros(rows,cols,CV_8UC1);        //显示水平投影结果
	for(i = 0;i < rows;i++)
	{
		uchar *ptr1 = horizontal.ptr<uchar>(i);
		for(j = 0;j < pixSum[i];j++)
			ptr1[j] = 255;
	}
	//namedWindow("horizontalProjection",0);
	//imshow("horizontalProjection",horizontal);
	//waitKey();
}

Mat basicOperation::getColorArea(Mat src,Mat &mask)
{
	int rows = mask.rows;
	int cols = mask.cols;
	int halfRow = rows / 4;
	int halfCol = cols / 4;
	//int halfRow = 0;
	//int halfCol = 0;
	int i;
	int left = 0,right = 0,down = 0,up = 0;
	vector<int> verticalPix;
	vector<int> horizontalPix;

	verticalProjection(mask,verticalPix);
	horizontalProjection(mask,horizontalPix);

	int vertiacalLength = verticalPix.size();
	int horizontalLength = horizontalPix.size();

	for(i = 0;i < vertiacalLength;i++)
	{
		if(verticalPix[i] > halfRow)
		{
			left = i;
			break;
		}
	}
	for(i = vertiacalLength - 1;i > -1;i--)
	{
		if(verticalPix[i] > halfRow)
		{
			right = i;
			break;
		}
	}
	for(i = 0;i < horizontalLength;i++)
	{
		if(horizontalPix[i] > halfCol)
		{
			down = i;
			break;
		}
	}
	for(i = horizontalLength - 1;i > -1;i--)
	{
		if(horizontalPix[i] > halfCol)
		{
			up = i;
			break;
		}
	}
	Mat colorArea;
	if(down > -1 && down < rows && up > -1 && up < rows && left > -1 && left < cols && right > -1 && right < cols)
	{
		Rect area(left,down,right-left,up-down);
		colorArea = src(area);
		mask = mask(area);
	}
	else
	{
		colorArea = Mat::zeros(1,1,CV_8UC1);
	}

	return colorArea;
}

Mat basicOperation::getColorMaskBlue(Mat src)
{
	Mat src_hsv;
	cvtColor(src, src_hsv, CV_BGR2HSV);

	vector<Mat> hsvSplit;
	split(src_hsv, hsvSplit);
	equalizeHist(hsvSplit[2], hsvSplit[2]);
	merge(hsvSplit, src_hsv);

	int rows = src.rows;
	int cols = src.cols*3;
	int maskCols = src.cols;
	Mat mask = Mat::zeros(rows, maskCols, CV_8UC1);
	int i,j,m;

	for (i = 0; i < rows; i++)
	{
		uchar* ptr = src_hsv.ptr<uchar>(i);
		uchar* ptrMask = mask.ptr<uchar>(i);
		for (j = 0, m = 0; j < cols; j = j + 3, m++)
		{
			int H = ptr[j];
			int S = ptr[j + 1];
			int V = ptr[j + 2];

			if (H > 84 && H < 125 && S > 60 && V > 40)
				ptrMask[m] = 255;
		}
	}
	return mask;
}

Mat basicOperation::getColorMaskYellow(Mat src)
{
	Mat src_hsv;
	cvtColor(src, src_hsv, CV_BGR2HSV);

	vector<Mat> hsvSplit;
	split(src_hsv, hsvSplit);
	equalizeHist(hsvSplit[2], hsvSplit[2]);
	merge(hsvSplit, src_hsv);

	int rows = src.rows;
	int cols = src.cols*3;
	int maskCols = src.cols;
	Mat mask = Mat::zeros(rows, maskCols, CV_8UC1);

	int i,j,m;


	for(i = 0;i < rows;i++)
	{
		uchar* ptr = src_hsv.ptr<uchar>(i);
		uchar* ptrMask = mask.ptr<uchar>(i);
		for(j  = 0,m = 0;j < cols;j = j+3,m++)
		{
			int H = ptr[j];
			int S = ptr[j+1];
			int V = ptr[j+2];

			if( H > 15 && H < 35 && S > 38 && V > 90)
				ptrMask[m] = 255;
		}
	}
	return mask;
}

int basicOperation::plateType(Mat src)
{
	Mat src_hsv;
	cvtColor(src, src_hsv, CV_BGR2HSV);

	vector<Mat> hsvSplit;
	split(src_hsv, hsvSplit);
	equalizeHist(hsvSplit[2], hsvSplit[2]);
	merge(hsvSplit, src_hsv);

	int rows = src.rows;
	int cols = src.cols*3;
	int i,j;

	int blue = 0,yellow = 0;
	for(i = 0;i < rows;i++)
	{
		uchar* ptr = src_hsv.ptr<uchar>(i);
		for(j = 0;j < cols;j = j+3)
		{
			int H = ptr[j];
			int S = ptr[j+1];
			int V = ptr[j+2];

			if(H > 84 && H < 125 && S > 60 && V > 40)
				blue++;
			if(H > 15 && H < 35 && S > 38 && V > 90)
				yellow++;
		}
	}
	if(blue > yellow)
		return 1;
	else
		return 2;
}

void basicOperation::getGrayFromYellow(Mat &image)
{
	Mat temp = image.clone();
	int rows = image.rows;
	int cols = image.cols;
	int i,j;

	for(i = 0;i < rows;i++)
	{
		uchar* ptr = image.ptr<uchar>(i);
		uchar* ptrT = temp.ptr<uchar>(i);
		for(j = 0;j < cols;j++)
			ptr[j] = 255 - ptrT[j];
	}
}
