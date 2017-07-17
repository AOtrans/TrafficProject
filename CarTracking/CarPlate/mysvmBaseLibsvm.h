#include "svm.h"
#include <iostream>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#ifndef MYSVMBASELIBSVM
#define MYSVMBASELIBSVM

class mysvmBaseLibsvm
{
private:
	struct svm_parameter param;               //SVM参数
	struct svm_model* model;                  //SVM训练好的model
	struct svm_problem problem;               //训练特征集
	vector<double> feature;                   //单个样本得到的特征
	int whichContruct;
	string type;
	string enTrainModel;
	string chTrainModel;
public:
	mysvmBaseLibsvm(vector<string> &path,string type);
	mysvmBaseLibsvm(string type);
	mysvmBaseLibsvm();
	~mysvmBaseLibsvm();
	void setType(string type);
	void setEnTrainModelPath(string enTrainModel);
	void setChTrainModelPath(string chTrainModel);
	void saveModel();                        //保存训练好的参数
	void loadModel();                        //加载训练好的参数
	void getFeature(Mat &plate);                 //获得特征
	void classify(vector<string> &path);     //根据路径判断该分类，确定目标输出
	void myLibsvmTrain();                    //训练
	double myLibsvmPredict(Mat &plate);       //预测
	int sumMatValue(const Mat& image);

	void setFreeModel();
};

#endif
