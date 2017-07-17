#include <opencv2/opencv.hpp>
#include <iostream>
#include "basicOperation.h"
#include "mysvmBaseLibsvm.h"

using namespace cv;
using namespace std;

class characterRecognition
{
private:
	mysvmBaseLibsvm mySvmCH;
	mysvmBaseLibsvm mySvmEN;
	string enTrainModel;
	string chTrainModel;
public:
	void setEnTrainModelPath(string enTrainModel);
	void setChTrainModelPath(string chTrainModel);
	void setCharRecognitionParam();
	string process(vector<Mat> &plateChar);
	void setFreeEN_CHModel();
};
