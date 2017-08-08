#include "colorImageEdgeDetect.h"

Mat colorImageEdgeDetect::RGB2HSI(Mat &image)
{
	Mat bgr[3];
	Mat hsi[3];
	split(image,bgr);
	split(image,hsi);

	hsi[0].convertTo(hsi[0],CV_32F);
	hsi[1].convertTo(hsi[1],CV_32F);
	hsi[2].convertTo(hsi[2],CV_32F);

	int i,j;
	int thra;
	double up = 0.0,down = 0.0;
	int rows = image.rows;
	int cols = image.cols;

	for(i = 0;i < rows;i++)
	{
		uchar *ptrb = bgr[0].ptr<uchar>(i);
		uchar *ptrg = bgr[1].ptr<uchar>(i);
		uchar *ptrr = bgr[2].ptr<uchar>(i);

		float *ptrh = hsi[0].ptr<float>(i);
		float *ptrs = hsi[1].ptr<float>(i);
		float *ptri = hsi[2].ptr<float>(i);
		for(j = 0;j < cols;j++)
		{
			up = (ptrr[j] - ptrg[j] + ptrr[j] - ptrb[j]) / 2.0;
			down = sqrt((float)((ptrr[j] - ptrg[j])*(ptrr[j] - ptrg[j]) + (ptrr[j] - ptrb[j])*(ptrg[j] - ptrb[j])));
			thra = (int)(180*acos(up / down)/3.14);

			if(ptrb[j] > ptrg[j])
				ptrh[j] = (360 - thra)*1.0/360;
			else
				ptrh[j] = thra*1.0/360;

			ptrs[j] = 1 - 3.0*myMin(ptrr[j],ptrg[j],ptrb[j])/(ptrr[j]+ptrg[j]+ptrb[j]);

			ptri[j] = (ptrr[j]+ptrg[j]+ptrb[j])*1.0/(3*255);

		}
	}

	Mat lastImage;

	merge(hsi,3,lastImage);

	return lastImage;
}

uchar colorImageEdgeDetect::myMin(uchar a,uchar b,uchar c)
{
	uchar min = -1;
	if(min > a)
		min = a;
	if(min > b)
		min = b;
	if(min > c)
		min =c;
	return min;
}

Mat colorImageEdgeDetect::HedgeV(Mat &Himage,Mat &Simage)                          //H分量边缘检测
{
	int rows = Himage.rows;
	int cols = Himage.cols;
	int i,j;
	Mat h = Mat::zeros(rows,cols,CV_32FC1);

	for(i = 1;i < rows-1;i++)
	{
		for(j = 1;j < cols-1;j++)
		{
			float a1 = Simage.at<float>(i-1,j-1);
			float a2 = Simage.at<float>(i-1,j+1);
			float a3 = Simage.at<float>(i,j-1);
			float a4 = Simage.at<float>(i,j+1);
			float a5 = Simage.at<float>(i+1,j-1);
			float a6 = Simage.at<float>(i+1,j+1);

			float b1 = Himage.at<float>(i-1,j-1);
			float b2 = Himage.at<float>(i-1,j+1);
			float b3 = Himage.at<float>(i,j-1);
			float b4 = Himage.at<float>(i,j+1);
			float b5 = Himage.at<float>(i+1,j-1);
			float b6 = Himage.at<float>(i+1,j+1);

			h.at<float>(i,j) = differ(a1,a2,b1,b2) + 2*differ(a3,a4,b3,b4) + differ(a5,a6,b5,b6);
		}
	}

	return h;
}

Mat colorImageEdgeDetect::SIedgeV(Mat &image)                         //S I 分量边缘检测
{
	int rows = image.rows;
	int cols = image.cols;
	int i,j;
	Mat si = Mat::zeros(rows,cols,CV_32FC1);

	for(i = 1;i < rows-1;i++)
	{
		for(j = 1;j < cols-1;j++)
		{
			float a1 = image.at<float>(i-1,j-1);
			float a2 = image.at<float>(i-1,j+1);
			float a3 = image.at<float>(i,j-1);
			float a4 = image.at<float>(i,j+1);
			float a5 = image.at<float>(i+1,j-1);
			float a6 = image.at<float>(i+1,j+1);

			si.at<float>(i,j) = abs(a1 - a2) + 2*abs(a3 - a4) + abs(a5 - a6);
		}
	}

	return si;
}

Mat colorImageEdgeDetect::HedgeH(Mat &Himage,Mat &Simage)              //H分量边缘检测（水平边缘检测）
{
	int rows = Himage.rows;
	int cols = Himage.cols;
	int i,j;
	Mat h = Mat::zeros(rows,cols,CV_32FC1);

	for(i = 1;i < rows-1;i++)
	{
		for(j = 1;j < cols-1;j++)
		{
			float a1 = Simage.at<float>(i-1,j-1);
			float a2 = Simage.at<float>(i+1,j-1);
			float a3 = Simage.at<float>(i-1,j);
			float a4 = Simage.at<float>(i+1,j);
			float a5 = Simage.at<float>(i-1,j+1);
			float a6 = Simage.at<float>(i+1,j+1);

			float b1 = Himage.at<float>(i-1,j-1);
			float b2 = Himage.at<float>(i+1,j-1);
			float b3 = Himage.at<float>(i-1,j);
			float b4 = Himage.at<float>(i+1,j);
			float b5 = Himage.at<float>(i-1,j+1);
			float b6 = Himage.at<float>(i+1,j+1);

			h.at<float>(i,j) = differ(a1,a2,b1,b2) + 2*differ(a3,a4,b3,b4) + differ(a5,a6,b5,b6);
		}
	}

	return h;
}

Mat colorImageEdgeDetect::SIedgeH(Mat &image)                         //S I 分量边缘检测（水平边缘检测）
{
	int rows = image.rows;
	int cols = image.cols;
	int i,j;
	Mat si = Mat::zeros(rows,cols,CV_32FC1);

	for(i = 1;i < rows-1;i++)
	{
		for(j = 1;j < cols-1;j++)
		{
			float a1 = image.at<float>(i-1,j-1);
			float a2 = image.at<float>(i+1,j-1);
			float a3 = image.at<float>(i-1,j);
			float a4 = image.at<float>(i+1,j);
			float a5 = image.at<float>(i-1,j+1);
			float a6 = image.at<float>(i+1,j+1);

			si.at<float>(i,j) = abs(a1 - a2) + 2*abs(a3 - a4) + abs(a5 - a6);
		}
	}

	return si;
}

float colorImageEdgeDetect::differ(float a1,float a2,float a3,float a4)
{
	float result = 0.0;
	float up = 0.0;

	result = (a1 + a2) / 2;
	up = abs(a3 - a4);
	result = result * sin(up / 2);

	return result;
}

Mat colorImageEdgeDetect::localIntensity(Mat &V,Mat &H)                                 //局部边缘强度，综合垂直，水平边缘检测
{
	int rows = V.rows;
	int cols = V.cols;
	int i,j;
	Mat result = Mat::zeros(rows,cols,CV_32FC1);

	for(i = 0;i < rows;i++)
	{
		float *ptrv = V.ptr<float>(i);
		float *ptrh = H.ptr<float>(i);
		float *ptr = result.ptr<float>(i);
		for(j = 0;j < cols;j++)
		{
			if(ptrv[j] < ptrh[j])
				ptr[j] = ptrh[j];
			else
				ptr[j] = ptrv[j];
		}
	}

	return result;
}

Mat colorImageEdgeDetect::colorIntensity(Mat &H,Mat &S,Mat &I,float p)
{
	int rows = H.rows;
	int cols = H.cols;
	int i,j;

	Mat colorIntensity = Mat::zeros(rows,cols,CV_32FC1);

	for(i = 0;i < rows;i++)
	{
		float *ptrh = H.ptr<float>(i);
		float *ptrs = S.ptr<float>(i);
		float *ptri = I.ptr<float>(i);
		float *ptr = colorIntensity.ptr<float>(i);

		for(j = 0;j < cols;j++)
		{
			ptr[j] = p*ptrh[j] + (1-p)*(ptrs[j] + ptri[j]) / 2;	
		}
	}

	return colorIntensity;
}

Mat colorImageEdgeDetect::To255(Mat &Image)
{
	Mat image = Image.clone();
	int rows = image.rows;
	int cols = image.cols;
	int i,j;

	Mat outputImage(rows,cols,CV_8UC1);

	for(i = 0;i < rows;i++)
	{
		uchar *ptr = outputImage.ptr<uchar>(i);
		float *ptr1 = image.ptr<float>(i);
		for(j = 0;j < cols;j++)
			ptr[j] = (uchar)(255*ptr1[j]);
	}
	//namedWindow("边缘检测图",0);
	//imshow("colorImageEdgeDetect 边缘检测图",outputImage);
	//waitKey();
	
	return outputImage;
}

float colorImageEdgeDetect::theMaxValue(Mat &mat)
{
	int rows = mat.rows;
	int cols = mat.cols;
	int i,j;
	float max = 0;

	for(i = 0;i < rows;i++)
	{
		float *ptr = mat.ptr<float>(i);
		for(j = 0;j < cols;j++)
			if(max < ptr[j])
				max = ptr[j];
	}
	return max;
}

void colorImageEdgeDetect::normalized(Mat &image)                                       //归一化
{
	int rows = image.rows;
	int cols = image.cols;
	int i,j;

	float max = theMaxValue(image);

	for(i = 0;i < rows;i++)
	{
		float *ptr = image.ptr<float>(i);
		for(j = 0;j < cols;j++)
			ptr[j] = ptr[j] / max;
	}
}

Mat colorImageEdgeDetect::edgeV(Mat &image)
{
	int rows = image.rows;
	int cols = image.cols;

	Mat hsi = RGB2HSI(image);
	Mat HSI[3];
	split(hsi,HSI);

	Mat hsiEdgeV[3];                //垂直边缘
	hsiEdgeV[0] = HedgeV(HSI[0],HSI[1]);
	hsiEdgeV[1] = SIedgeV(HSI[1]);
	hsiEdgeV[2] = SIedgeV(HSI[2]);

	float p = 0.5;                             //权值,未测试好的阈值？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？
	Mat intensity = colorIntensity(hsiEdgeV[0],hsiEdgeV[1],hsiEdgeV[2],p);
	normalized(intensity);

	Mat outputImage = To255(intensity);

	return outputImage;
}

Mat colorImageEdgeDetect::edgeVH(Mat &image)
{
	int rows = image.rows;
	int cols = image.cols;

	Mat hsi = RGB2HSI(image);
	Mat HSI[3];
	split(hsi,HSI);

	Mat hsiEdgeV[3];                //垂直边缘
	hsiEdgeV[0] = HedgeV(HSI[0],HSI[1]);
	hsiEdgeV[1] = SIedgeV(HSI[1]);
	hsiEdgeV[2] = SIedgeV(HSI[2]);

	Mat hsiEdgeH[3];               //水平边缘
	hsiEdgeH[0] = HedgeH(HSI[0],HSI[1]);
	hsiEdgeH[1] = SIedgeH(HSI[1]);
	hsiEdgeH[2] = SIedgeH(HSI[2]);

	Mat hsiEdge[3];                 //水平，垂直边缘综合
	hsiEdge[0] = localIntensity(hsiEdgeH[0],hsiEdgeV[0]);
	hsiEdge[1] = localIntensity(hsiEdgeH[1],hsiEdgeV[1]);
	hsiEdge[2] = localIntensity(hsiEdgeH[2],hsiEdgeV[2]);

	float p = 0.5;                             //权值,未测试好的阈值？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？
	Mat intensity = colorIntensity(hsiEdge[0],hsiEdge[1],hsiEdge[2],p);
	normalized(intensity);

	Mat outputImage = To255(intensity);

	return outputImage;
}

Mat colorImageEdgeDetect::process(Mat &image)
{
	Mat outputImage = edgeV(image);                //垂直边缘检测
	//Mat outputImage = edgeVH(image);                //垂直水平边缘检测

	threshold(outputImage,outputImage,40,255,THRESH_BINARY);    //二值化阈值选取，隐患？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？
	//imshow("二值化图像",outputImage);
	//waitKey();
	return outputImage;
}



//阈值问题
//1.强度权值
//2.二值化阈值
