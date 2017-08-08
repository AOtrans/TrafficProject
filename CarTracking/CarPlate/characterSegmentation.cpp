#include "characterSegmentation.h"
#include "getConnectedDomain.h"

bool characterSegmentation::horizontalEdgeRemove(Mat &image,int &up,int &down)
{
	int rows = image.rows;
	int cols = image.cols;
	vector<int> count;
	int i,j;

	for(i = 0;i < rows;i++)
	{
		uchar *ptr = image.ptr<uchar>(i);
		count.push_back(0);
		for(j = 0;j < cols-1;j++)
		{
			if(ptr[j+1]-ptr[j] != 0)
				count[i]++;
		}
	}

	for(i = 0;i < rows;i++)                            //第一轮清除上下边界
	{
		uchar *ptr = image.ptr<uchar>(i);
		if(count[i] < 13)                              //单纯把字母当成竖线，最少的跳变次数为14
		{
			for(j = 0;j < cols;j++)
				ptr[j] = 0;
		}
	}
	
	vector<int> pixSum;
	bso.horizontalProjection(image,pixSum);
	//缩小row        是必要的，避免车牌上下宽度过大的影响                    
	int tempUp = 0;
	int tempDown = 0;
	int pixSumLength = pixSum.size();
	for(i = 0;i < pixSumLength;i++)
	{
		if(pixSum[i] != 0)
		{
			if(i == 0)
			{
				tempDown = i;
				break;
			}
			else
			{
				tempDown = i-1;                   
				break;
			}
		}
	}
	for(i = pixSumLength-1;i > 0;i--)
	{
		if(pixSum[i] != 0)
		{
			if(i == pixSumLength-1)
			{
				tempUp = pixSumLength;
				break;
			}
			else
			{
				tempUp = i+1;                    
				break;
			}
		}
	}

	if(tempUp - tempDown < 3)              //阈值BUG？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？
		return false;
	if(tempUp + 1 < rows)
		tempUp = tempUp + 1;
	image = image.rowRange(tempDown,tempUp);

	pixSum.clear();
	bso.horizontalProjection(image,pixSum);
	int newRows = image.rows;
	int newCols = image.cols;

	pixSumLength = pixSum.size();
	//第二轮清除上下边界
	int statePre = pixSum[0];
	vector<int> changePosition;
	bool isTrue = false;
	changePosition.push_back(0);                   //习惯压入起始点也是特殊情况的容错
	for(i = 1;i < pixSumLength;i++)
	{
		if(statePre == 0 && pixSum[i] != 0)      // || (thePreState != 0 && sumPix[i] == 0))
		{
			changePosition.push_back(i-1);
			statePre = pixSum[i];
		}
		else if(statePre != 0 && pixSum[i] == 0)
		{
			changePosition.push_back(i);
			statePre = pixSum[i];
		}
		else
			continue;
	}
	changePosition.push_back(i-1);                 //习惯压入末点也是特殊情况的容错
	int changePositionLength = changePosition.size();

	int imageHalf = image.rows / 3;                                      //倾斜校正后row可能变大了，BUG？？？？？？除BUG方法，根据第一次去除边界后，根据有效像素点对图片进行从新切割，缩小row
	for(i = 0;i < changePositionLength - 1;i++)
	{
		int mid = (changePosition[i+1] + changePosition[i]) / 2;
		int downChange = changePosition[i];
		int upChange = changePosition[i+1];
		if(upChange - downChange > imageHalf && pixSum[mid] != 0)
		{
			if(upChange+1 < newRows)
				upChange++;
			image = image.rowRange(downChange,upChange);
			down = tempDown + changePosition[i];
			up = tempDown + changePosition[i+1];
			break;
		}
	}
	newRows = image.rows;
	newCols = image.cols;
	Mat imageExpand = Mat::zeros(newRows+1,newCols,CV_8UC1);
	for(i = 0;i < newRows;i++)
	{
		uchar* ptr = image.ptr<uchar>(i);
		uchar* ptre = imageExpand.ptr<uchar>(i);
		for(j = 0;j < newCols;j++)
			ptre[j] = ptr[j];
	}
	image = imageExpand.clone();

	if(up <= down)
		return false;

	if(up >= rows)
		up = rows-1;

	return true;
}

bool characterSegmentation::findZeroPosition(vector<int> &sumPix,int charHight,struct charPos charPosition[])          
{
	vector<int> changePosition;

	int realCharLength = (int)(charHight / 2);        //字符扩充大小时阈值
	int halfCharLength = (int)(charHight / 4);        //过滤1的宽度
	int oneHight = (int)(charHight / 2);            //判断字符高度，主要用于判断1的情况
	int charMaxLength = charHight;         //字符最大宽度，普通字符宽度1.5倍
	//int oneBlankLenth = (int)(charHight / 4);          //字符1和临近字符空白区域的距离28  字符宽度45
	int minBlankLenth = (int)(charHight*0.18);       //普通字符之间的距离，不包括1的情况  这个计算有问题

	int thePreState = sumPix[0];
	int length = sumPix.size();
	int i = 0,j;

	changePosition.push_back(0);                       //初始位置压栈，避免字符贴近边缘的情况，其实不保存也没事，因为关键汉字定位是通过其它字符位置来定位的
	for(i = 1;i < length;i++)                          //保存所有从0到非0变化的临界点
	{
		if(thePreState == 0 && sumPix[i] != 0)      // || (thePreState != 0 && sumPix[i] == 0))
		{
			changePosition.push_back(i-1);
			thePreState = sumPix[i];
		}
		else if(thePreState != 0 && sumPix[i] == 0)
		{
			changePosition.push_back(i);
			thePreState = sumPix[i];
		}
		else
			continue;
	}

	//if(sumPix[length-1] != 0)
	changePosition.push_back(length-1);                                   //特殊情况，最后一个字符紧贴边界  给到颜色区域后，这种情况不一定存在了，1的情况可以简化处理了

	int charCount = 0;
	int changePositionSum = changePosition.size();
	for(i = changePositionSum - 1;i > 1;i--)                          //找前六个字符
	{
		if(changePosition[i] - changePosition[i - 1] - 1 > halfCharLength)                      //容错BUG 貌似可以选字符宽度的一半，1字符就当特殊字符处理，它的宽度也达不到普通字符的一半
		{
			if(changePosition[i] - changePosition[i - 1] - 1 > charMaxLength && charCount == 0)                   //当宽度大于字符宽度的1.5倍时，判断为非字符区域  charCount < 4 中间字符有污染的情况
				continue;
			int mid = (changePosition[i] + changePosition[i-1]) / 2;
			if(sumPix[mid] == 0)
				continue;
			charPosition[charCount].right = changePosition[i];
			charPosition[charCount].left = changePosition[i-1];
			charCount++;
			if(charCount == 6)
				break;
		}
		else
		{
			int mid = (changePosition[i] + changePosition[i-1]) / 2;
			int maxPix = 0;
			for(j = changePosition[i-1];j < changePosition[i];j++)
			{
				if(maxPix < sumPix[j])
				{
					maxPix = sumPix[j];
					mid = j;
				}
			}
			if(sumPix[mid] > oneHight)
			{
				if(changePosition[i-1] - changePosition[i-2] + 1 > minBlankLenth)         //minBlankLenth
				{
					if(charCount < 1)
					{
						if(i+1 == changePositionSum || changePosition[i+1] - changePosition[i] - 1 < minBlankLenth)
							continue;
					}

					charPosition[charCount].right = changePosition[i];
					charPosition[charCount].left = changePosition[i-1];
					charCount++;
					if(charCount == 6)                                                         //同样关键
						break;
				}
				else 
				{
					if(i+1 < changePositionSum && changePosition[i+1] - changePosition[i] + 1 > minBlankLenth)      //charCount > 1 去除为边界的情况
					{
						if(charCount > 0)
						{
							charPosition[charCount].right = changePosition[i];
							charPosition[charCount].left = changePosition[i-1];
							charCount++;
							if(charCount == 6)                                                         //同样关键
								break;
						}	
					}
					else
					{
						continue;
					}
				}
			}
		}
	}

	int sumPixSize = sumPix.size();
	for(i = 0;i < 6;i++)            //投影分割失败时，为按比例分割做准备
	{
		if(charPosition[i].left > sumPixSize || charPosition[i].left < 0 || charPosition[i].right > sumPixSize || charPosition[i].right < 0)
		{
			charPosition[i].left = -1;
			charPosition[i].right = -1;
		}
	}

	//以下代码
	if(!getRightPosition(charPosition,charMaxLength,charHight,length))
		return false;

	for(i = 0;i < 6;i++)            //判断分割是否成功
	{
		if(charPosition[i].left > sumPixSize || charPosition[i].left < 0 || charPosition[i].right > sumPixSize || charPosition[i].right < 0)
		{
			return false;
		}
	}

	//找汉字的分割线
	int twoCharDistance = (charPosition[1].right + charPosition[1].left - charPosition[2].right - charPosition[2].left) / 2;

	charPosition[6].right = (charPosition[5].right + charPosition[5].left) / 2 - twoCharDistance;
	charPosition[6].left = charPosition[6].right;


	for(i = 0;i < 7;i++)             //增加1的宽度                                    
	{
		while(charPosition[i].right - charPosition[i].left < realCharLength)                   
		{
			charPosition[i].right++;
			charPosition[i].left--;
			if(charPosition[i].right > length-1)                                    //防止右边界字符越界
				charPosition[i].right = length - 1;
			if(charPosition[i].left < 0)                                            //防止左边界字符越界                                
				charPosition[i].left = 0;
		}
	}

	return true;
}

bool characterSegmentation::getRightPosition(struct charPos charPosition[],int charMaxLength,int charHight,int plateLength)
{
	double charRelativePosition[6] = {0.926,0.797,0.667,0.538,0.408,0.228};         //车牌除了汉字，其它字符相对左边缘的位置比例
	struct charPos tempCharPosition[6];
	int charMidDist = charHight*57/90;
	int charMaxDist = charHight*79/90;
	bool charLengthIsRight[6] = {true,true,true,true,true,true};
	int i,j,m;

	for(i = 0;i < 6;i++)
	{
		if(charPosition[i].left == -1)
		{
			charLengthIsRight[i] = false;
			continue;
		}
		if(charPosition[i].right - charPosition[i].left > charMaxLength)
		{
			charLengthIsRight[i] = false;
		}
	}
	int falseCount = 0;
	int trueCount = 0;
	for(i = 0;i < 6;i++)
	{
		if(!charLengthIsRight[i])
			falseCount++;
	}
	for(i = 0;i < 6;i++)
	{
		if(charLengthIsRight[i])
			trueCount++;
	}
	if(falseCount == 6)
		return false;
	if(trueCount == 6)
		return true;

	//改进按比例分割
	for(i = 0;i < 6;i++)
	{
		tempCharPosition[i].left = 4000;
		tempCharPosition[i].right = 5000;
	}

	for(i = 0;i < 6;i++)
	{
		if(charLengthIsRight[i])
		{
			int currentCharPos = (charPosition[i].left + charPosition[i].right) / 2;
			double relativePositin = currentCharPos*1.0 / plateLength;
			for(m = 0;m < 6;m++)
			{
				double excatError = abs(charRelativePosition[m] - relativePositin);
				if(excatError <= 0.06)
				{
					tempCharPosition[m].left = currentCharPos;
					tempCharPosition[m].right = currentCharPos;
				}
			}
		}
	}
	bool charPositionIsRight[6] = {true,true,true,true,true,true};
	for(i = 0;i < 6;i++)
	{
		if(tempCharPosition[i].left == 4000)
			charPositionIsRight[i] = false;
	}

	for(i = 0;i < 5;i++)
	{
		if(charPositionIsRight[i] && !charPositionIsRight[i+1])
		{
			int left = tempCharPosition[i].left;
			if(i < 4)
				left -= charMidDist;
			else
				left -= charMaxDist;
			tempCharPosition[i+1].left = left;
			tempCharPosition[i+1].right = left;
			charPositionIsRight[i+1] = true;
		}
	}

	for(i = 5;i > 0;i--)
	{
		if(charPositionIsRight[i] && !charPositionIsRight[i-1])
		{
			int right = tempCharPosition[i].right;
			if(i == 5)
				right += charMaxDist;
			else
				right += charMidDist;
			tempCharPosition[i-1].right = right;
			tempCharPosition[i-1].left = right;
			charPositionIsRight[i-1] = true;
		}
	}

	for(i = 0;i < 6;i++)
	{
		int mid = (tempCharPosition[i].left + tempCharPosition[i].right) / 2;
		charPosition[i].left = mid;
		charPosition[i].right = mid;
	}

	return true;
}

void characterSegmentation::getTheChar(struct charPos* charPosition,int n,Mat &image,vector<Mat> &pateChar)
{
	int i;

	for(i = n-1;i > -1;i--)          //切割字符
	{
		pateChar.push_back(image.colRange(charPosition[i].left,charPosition[i].right));
	}
}

void characterSegmentation::getPoint(vector<Point> &point,Mat &image)
{
	getConnectedDomain getDomain;
	vector<vector<int>> ableRectangle;
	getDomain.findDomainBaseStrokeBaseOnRonny(image,ableRectangle);

	int ableRectangleLength = ableRectangle.size();
	int i;
	int down = image.rows / 4;
	int up = image.rows*3 / 4;
	int cols = image.cols;

	for(i = 0;i < ableRectangleLength;i++)
	{
		//位置
		int x = (ableRectangle[i][2] + ableRectangle[i][3]) / 2;
		int y = (ableRectangle[i][0] + ableRectangle[i][1]) / 2;

		if(y < down || y > up)
			continue;

		//比例 面积
		int xLength = ableRectangle[i][3] - ableRectangle[i][2];
		int yLength = ableRectangle[i][1] - ableRectangle[i][0];
		int area = xLength * yLength;

		if(xLength < 1)
			continue;
		double xyRate = yLength*1.0 / xLength;
		if(xyRate < 1)
			continue;
		if(xyRate > 3)
		{
			double position = x*1.0 / cols;
			if(position < 0.25 || position > 0.97 || area < 70)
				continue;
			else
			{
				Point tempPoint(x,y);
				point.push_back(tempPoint);
				continue;
			}
		}
	
		if(area < 200 || area > 650)
			continue;

		Point tempPoint(x,y);
		point.push_back(tempPoint);
	}
}

bool characterSegmentation::getDegree(Mat &image,double &degree)
{
	vector<Point> point;
	getPoint(point,image);

	if(point.empty())
		return false;

	Vec4f lineParam;
	fitLine(point,lineParam,CV_DIST_L2,0,0.01,0.01);

	double lineRate = lineParam[1] / lineParam[0];

	degree = atan(-lineRate)*180 / 3.14;

	return true;
}

Mat characterSegmentation::rotateImage(Mat img, double degree)
{
	degree = -degree;//warpAffine默认的旋转方向是逆时针，所以加负号表示转化为顺时针
	double angle = degree  * CV_PI / 180.; // 弧度  
	double a = sin(angle), b = cos(angle);
	int width = img.cols;
	int height = img.rows;
	int width_rotate = int(height * fabs(a) + width * fabs(b));
	int height_rotate = int(width * fabs(a) + height * fabs(b));
	//旋转数组map
	// [ m0  m1  m2 ] ===>  [ A11  A12   b1 ]
	// [ m3  m4  m5 ] ===>  [ A21  A22   b2 ]
	float map[6];
	Mat map_matrix = Mat(2, 3, CV_32F, map);
	// 旋转中心
	CvPoint2D32f center = cvPoint2D32f(width / 2, height / 2);
	CvMat map_matrix2 = map_matrix;
	cv2DRotationMatrix(center, degree, 1.0, &map_matrix2);//计算二维旋转的仿射变换矩阵
	map[2] += (width_rotate - width) / 2;
	map[5] += (height_rotate - height) / 2;
	Mat img_rotate;
	//对图像做仿射变换
	//CV_WARP_FILL_OUTLIERS - 填充所有输出图像的象素。
	//如果部分象素落在输入图像的边界外，那么它们的值设定为 fillval.
	//CV_WARP_INVERSE_MAP - 指定 map_matrix 是输出图像到输入图像的反变换，
	warpAffine(img, img_rotate, map_matrix, Size(width_rotate, height_rotate), 1, 0, 0);
	return img_rotate;
}

void characterSegmentation::plateVerticalCorrection(Mat &image,double &degree)
{
	vector<int> pBlankSum;
	vector<int> nBlankSum;
	int i;
	int max = 0;

	degree = 0.0;
	for(i = 0;i < 20;i++)
	{
		Mat theRotateImage = verticalCorrection(image,degree);

		vector<int> pixSum;
		bso.verticalProjection(theRotateImage,pixSum);
		int totalBlankPix = BlankPixSum(pixSum);
		pBlankSum.push_back(totalBlankPix);
		degree = degree + 0.5;
	}

	degree = 0.0;
	for(i = 0;i < 20;i++)
	{
		Mat theRotateImage = verticalCorrection(image,degree);

		vector<int> pixSum;
		bso.verticalProjection(theRotateImage,pixSum);
		int totalBlankPix = BlankPixSum(pixSum);
		nBlankSum.push_back(totalBlankPix);
		degree = degree - 0.5;
	}

	for(i = 0;i < pBlankSum.size();i++)
	{
		if(pBlankSum[i] > max)
		{
			max = pBlankSum[i];
			degree = 0.5*i;
		}
	}

	for(i = 0;i < nBlankSum.size();i++)
	{
		if(nBlankSum[i] > max)
		{
			max = nBlankSum[i];
			degree = -0.5*i;
		}
	}
}

int characterSegmentation::BlankPixSum(vector<int> &pixSum)
{
	int i;
	int length = pixSum.size();
	int total = 0;

	for(i = 0;i < length;i++)
	{
		if(pixSum[i] == 0)
			total++;
	}

	return total;
}

Mat characterSegmentation::verticalCorrection(Mat &image,double degree)                 //从上往下看，绕点从左往右转 degree为正，从右往左degree为负
{
	double angle = degree  * CV_PI / 180.;
	int rows = image.rows;
	int cols = image.cols;
	int m,n;
	int col;
	double tempCol;
	double tanValue = tan(angle);

	Mat crrectImage = Mat::zeros(rows,cols,CV_8UC1);
	for(m = 0;m < rows;m++)
	{
		uchar *ptr = image.ptr<uchar>(m);
		uchar *ptrCrrect = crrectImage.ptr<uchar>(m);
		for(n = 0;n < cols;n++)
		{
			if(ptr[n] != 0)
			{
				tempCol = n + m*tanValue;
				if(tempCol - (int)tempCol > 0.5)
					col = (int)tempCol + 1;
				else
					col = (int)tempCol;

				if(degree > 0 && col >= cols)
					col = cols-1;
				else if(degree < 0 && col < 0)
					col = 0;

				ptrCrrect[col] = ptr[n];
			}
		}
	}

	return crrectImage;
}

void testVerticalProjection(Mat &image)
{
	int rows = image.rows;
	int cols = image.cols;
	int i,j;
	vector<int> pixSum;
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
	imshow("testVerticalProjection",vertical);
	waitKey();
}

void characterSegmentation::grayscaleStretching(Mat &image)
{
	//判断图像读取是否有问题
	if (!image.data)
	{
		cout << "image read is error!" << endl;
		return;
	}

	//处理图像
	int data_max = 255, data_min = 0;
	int nl = image.rows;
	int nc = image.cols * image.channels();
	if (image.isContinuous())
	{
		nc = nc * nl;
		nl = 1;
	}
	uchar *data;
	int i, j;

	getStretchMinMax(image, data_min, data_max);

	//cout << "data_max:" << data_max << " data_min:" << data_min << endl;

	int temp = data_max - data_min;
	for (j = 0; j < nl; j++)
	{
		data = image.ptr<uchar>(j);
		for (i = 0; i < nc; i++)
		{
			if (data[i] < data_min)
				data[i] = 0;
			else if (data[i] > data_max)
				data[i] = 255;
			else
				data[i] = (data[i] - data_min) * 255 / temp;
		}
	}
}

Mat characterSegmentation::onlyRChannalStretchingBlue(Mat image)
{
	Mat bgr[3];
	split(image, bgr);

	grayscaleStretching(bgr[2]);

	return bgr[2];
}

Mat characterSegmentation::onlyRChannalStretchingYellow(Mat image)
{
	Mat bgr[3];
	split(image, bgr);

	bso.getGrayFromYellow(bgr[2]);

	grayscaleStretching(bgr[2]);

	return bgr[2];
}

void characterSegmentation::getStretchMinMax(Mat image,int &min,int &max)
{
	int rows = image.rows;
	int cols = image.cols*image.channels();
	int i, j;

	if (image.isContinuous())
	{
		cols = rows * cols;
		rows = 1;
	}
	int grayScale[256] = { 0 };
	double downScale[256] = { 0 };
	double upScale[256] = { 0 };

	for (i = 0; i < rows; i++)
	{
		uchar* ptr = image.ptr<uchar>(i);
		for (j = 0; j < cols; j++)
		{
			grayScale[ptr[j]]++;
		}
	}
	int total = 0;
	int totalGray = rows*cols;
	for (i = 0; i < 256; i++)
	{
		total += grayScale[i];
		downScale[i] = total*1.0 / totalGray;
	}
	total = 0;
	for (i = 255; i > -1; i--)
	{
		total += grayScale[i];
		upScale[i] = total*1.0 / totalGray;
	}

	for (i = 0; i < 256; i++)
	{
		if (downScale[i] > 0.5)           //阈值
		{
			min = i;
			break;
		}
	}

	for (i = 255; i > -1; i--)
	{
		if (upScale[i] > 0.1)             //阈值
		{
			max = i;
			break;
		}
	}
}

double characterSegmentation::charDensity(Mat &image)
{
	int rows = image.rows;
	int cols = image.cols;
	int nonZeroPix = 0;
	int totalPix = rows*cols;
	int i,j;

	for(i = 0;i < rows;i++)
	{
		uchar* ptr = image.ptr<uchar>(i);
		for(j = 0;j < cols;j++)
		{
			if(ptr[j] != 0)
				nonZeroPix++;
		}
	}

	double result = nonZeroPix*1.0 / totalPix;

	return result;
}

void characterSegmentation::verticalEdgeRemove(Mat &image,int &left,int &right)
{
	vector<int>sumPix;
	bso.verticalProjection(image,sumPix);

	vector<int> changePosition;
	int charHight = image.rows;
	int halfCols = image.cols / 2;
	int halfRows = charHight / 2;
	int realPlateLength = (int)(charHight*4.9);

	int halfCharLength = (int)(charHight / 4);        
	int charMaxLength = charHight*2 / 3;      
	int horizontalBlank = charHight*3 / 8;

	int thePreState = sumPix[0];
	int length = sumPix.size();
	int i = 0,j;

	changePosition.push_back(0);                       
	for(i = 1;i < length;i++)                         
	{
		if(thePreState == 0 && sumPix[i] != 0)     
		{
			changePosition.push_back(i-1);
			thePreState = sumPix[i];
		}
		else if(thePreState != 0 && sumPix[i] == 0)
		{
			changePosition.push_back(i);
			thePreState = sumPix[i];
		}
		else
			continue;
	}
	changePosition.push_back(length-1);                                  

	int changePositionSum = changePosition.size();
	
	for(i = changePositionSum - 1;i > 1;i--)                         
	{
		int leftBorder = changePosition[i-1];
		int rightBorder = changePosition[i];

		if(leftBorder < halfCols)
			break;

		int isCharLength = rightBorder - leftBorder - 1;
		if(isCharLength < 0)
			continue;

		int mid = (leftBorder + rightBorder) / 2;
		if(sumPix[mid] == 0)
			continue;

		Mat isChar = image.colRange(leftBorder,rightBorder);

		if(isCharLength > halfCharLength)
		{
			Mat isChar = image.colRange(leftBorder,rightBorder);
			double density = charDensity(isChar);
			if(density > 0.85)
			{
				right = leftBorder;
				continue;
			}
		}
		else
			continue;

		if(!charChangeSum(isChar))
		{
			right = leftBorder;
			continue;
		}

		vector<int> verticalPix;
		bso.verticalProjection(isChar,verticalPix);
		int hightAve = charHight*2 / 3;
		int hightCount = verticalHightSum(verticalPix,hightAve);
		if(hightCount > halfCharLength)
		{
			right = leftBorder;
			continue;
		}

		vector<int> horizontalPix;
		bso.horizontalProjection(isChar,horizontalPix);
		int blank = horizontalBlankSum(horizontalPix);
		if(blank > horizontalBlank)
		{
			right = leftBorder;
			continue;
		}

		break;                           //所有条件不跳转，着终止循环
	}

	for(i = 0;i < changePositionSum -1;i++)
	{
		int leftBorder = changePosition[i];
		int rightBorder = changePosition[i+1];

		if(rightBorder > halfCols)
			break;

		int isCharLength = rightBorder - leftBorder - 1;
		if(isCharLength < 0)
			continue;

		int mid = (leftBorder + rightBorder) / 2;
		if(sumPix[mid] == 0)
			continue;
		
		if(isCharLength < halfCharLength)
			continue;
		
		Mat isChar = image.colRange(leftBorder,rightBorder);
		if(!charChangeSum(isChar))
		{
			left = rightBorder;
			continue;
		}

		vector<int> verticalPix;
		bso.verticalProjection(isChar,verticalPix);
		int hightAve = charHight*2 / 3;
		int hightCount = verticalHightSum(verticalPix,hightAve);
		if(hightCount > halfCharLength)
		{
			if(right - rightBorder < realPlateLength)
				continue;
			left = rightBorder;
			continue;
		}
	}
	for(i = left;i < halfCols;i++)
	{
		if(sumPix[i] == 0)
			left = i;
		else
			break;
	}
	if(right < left)
		return;
	image = image.colRange(left,right);
}

int characterSegmentation::horizontalBlankSum(vector<int> horizontalPix)
{
	int blank = 0;
	int horizontalPixLength = horizontalPix.size();
	int i;
	for(i  = 0;i < horizontalPixLength;i++)
		if(horizontalPix[i] == 0)
			blank++;

	return blank;
}

int characterSegmentation::verticalHightSum(vector<int> verticalPix,int hight)
{
	int count = 0;
	int verticalHightLength = verticalPix.size();
	int i;
	for(i = 0;i < verticalHightLength;i++)
	{
		if(verticalPix[i] > hight)
			count++;
	}
	return count;
}

bool characterSegmentation::charChangeSum(Mat &image)
{
	int rows = image.rows;
	int cols = image.cols;
	int i,j;
	int change = 0;
	vector<int> horizontal;
	vector<int> vertical;

	for(i = 0;i < rows;i++)
	{
		horizontal.push_back(0);
		uchar* ptr = image.ptr<uchar>(i);
		for(j = 1;j < cols;j++)
		{
			if(ptr[j] != ptr[j-1])
				horizontal[i]++;
		}
	}

	for(i = 0;i < cols;i++)
	{
		vertical.push_back(0);
		for(j = 1;j < rows;j++)
		{
			if(image.at<uchar>(j,i) != image.at<uchar>(j-1,i))
				vertical[i]++;
		}
	}

	int horizontalLength = horizontal.size();
	int verticalLength = vertical.size();

	for(i = 0;i < horizontalLength;i++)
	{
		if(horizontal[i] > 2)
			change++;
	}
	for(i = 0;i < verticalLength;i++)
	{
		if(vertical[i] > 2)
			change++;
	}
	if(change > 0)
		return true;
	else
		return false;
}

bool characterSegmentation::process(Mat &Image,vector<Mat> &pateChar)
{
	//imshow("车牌",Image);
	//waitKey();

	Mat image;
	Mat grayImageForChar = Image.clone();

	int carPlateType = bso.plateType(Image);

	Mat imageGray;
	int T;
	if(carPlateType == 1)
	{
		Mat myMask;

		myMask = bso.getColorMaskBlue(Image);

		int arry[3][3] = {1,1,1,1,1,1,1,1,1};
		Mat myKernel(3,3,CV_8UC1,arry);
		morphologyEx(myMask,myMask,MORPH_CLOSE,myKernel,Point(-1,-1),1);
		morphologyEx(myMask,myMask,MORPH_OPEN,myKernel,Point(-1,-1),1);

		image = bso.getColorArea(Image,myMask);

		grayImageForChar = image.clone();

		int testImageRows = image.rows;
		if(testImageRows < 2)
			return false;

		image = onlyRChannalStretchingBlue(image);

		grayImageForChar = onlyRChannalStretchingBlue(grayImageForChar);


		T = bso.myOSTU(image);
		threshold(image,imageGray,T,255,THRESH_BINARY);

	} 
	else if(carPlateType == 2)
	{
		Mat myMask;
		myMask = bso.getColorMaskYellow(Image);

		int arry[3][3] = {1,1,1,1,1,1,1,1,1};
		Mat myKernel(3,3,CV_8UC1,arry);
		morphologyEx(myMask,myMask,MORPH_CLOSE,myKernel,Point(-1,-1),1);
		morphologyEx(myMask,myMask,MORPH_OPEN,myKernel,Point(-1,-1),1);

		image = bso.getColorArea(Image,myMask);

		grayImageForChar = image.clone();

		int testImageRows = image.rows;
		if(testImageRows < 2)
			return false;

		image = onlyRChannalStretchingYellow(image);

		grayImageForChar = onlyRChannalStretchingYellow(grayImageForChar);

		T = bso.myOSTU(image);
		threshold(image,imageGray,T,255,THRESH_BINARY);

	}
	int imageCols = imageGray.cols;
	double colsRate = 136.0 / imageCols;              //固定车牌长度136
	int imageRows = (int)(image.rows * colsRate);

	resize(imageGray,imageGray,Size(136,imageRows));                 //调整大小时必须需的
	resize(grayImageForChar,grayImageForChar,Size(136,imageRows));
	T = bso.myOSTU(imageGray);
	threshold(imageGray,imageGray,T,255,THRESH_BINARY);
	
	double degree = 0.0;
	//倾斜校正
	if(getDegree(imageGray,degree))
	{
		grayImageForChar = rotateImage(grayImageForChar,degree);                              //原图旋转
		imageGray = rotateImage(imageGray,degree);
	}
	
	T = bso.myOSTU(imageGray);
	threshold(imageGray,imageGray,T,255,THRESH_BINARY);

	int up = 1,down = 0;
	if(!horizontalEdgeRemove(imageGray,up,down))           //水平边缘去除
		return false;
	if(up > down)
		grayImageForChar = grayImageForChar.rowRange(down,up); //原图去除上下边框

	plateVerticalCorrection(imageGray,degree);                       //垂直校正
	grayImageForChar = verticalCorrection(grayImageForChar,degree);                        //原图旋转
	imageGray = verticalCorrection(imageGray,degree);

	//按比例重新调整车牌大小
	imageCols = imageGray.cols;
	imageRows = (int)(imageCols*0.2);
	resize(imageGray,imageGray,Size(imageCols,imageRows));                 //调整大小时必须需的
	resize(grayImageForChar,grayImageForChar,Size(imageCols,imageRows));
	T = bso.myOSTU(imageGray);
	threshold(imageGray,imageGray,T,255,THRESH_BINARY);

	int left = 0,right = imageGray.cols - 1;
	verticalEdgeRemove(imageGray,left,right);
	if(right > left)
		grayImageForChar = grayImageForChar.colRange(left,right);

	//按比例重新调整车牌大小
	imageRows = imageGray.rows;
	imageCols = (int)(imageRows*4.9);
	resize(imageGray,imageGray,Size(imageCols,imageRows));                 //调整大小时必须需的
	resize(grayImageForChar,grayImageForChar,Size(imageCols,imageRows));
	T = bso.myOSTU(imageGray);
	threshold(imageGray,imageGray,T,255,THRESH_BINARY);

	vector<int>sumPix;
	bso.verticalProjection(imageGray,sumPix);

	struct charPos charPosition[7];

	for(int i = 0;i < 7;i++)                //初始化大于图片宽度
	{
		charPosition[i].left = 4000;
		charPosition[i].right = 5000;
	}

	if(!findZeroPosition(sumPix,imageGray.rows,charPosition))
		return false;

	getTheChar(charPosition,7,grayImageForChar,pateChar);
	
	return true;
}
