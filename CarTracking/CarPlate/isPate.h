#include <opencv2/opencv.hpp>
#include <fstream>

using namespace cv;
using namespace std;

#ifndef ISPATE
#define ISPATE

class isPate                               //判断是否为车牌类
{
private:
	CvSVM classifier;  
	CvSVMParams SVM_params;
	Mat feture;
	Mat label;
	string plateSvmTrainModel;
public:
	isPate();
	void setTrainModelPath(string plateSvmTrainModel);
	void getFeture(vector<string> &path);    //得到训练特征
	void svmTrain();                         //训练函数
	int svmPredict(Mat &pate);               //预测函数
	void saveSvm();                          //SVM参数保存
	void readSvm();                           //读取训练好的SVM参数
};

#endif
