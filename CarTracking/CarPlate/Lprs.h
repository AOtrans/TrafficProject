#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>

#include "licensePlateLocation.h"
#include "characterSegmentation.h"
#include "characterRecognition.h"

using namespace cv;
using namespace std;

class Lprs                //License plate recognition system
{
private:
	licensePlateLocation lpl;
	characterSegmentation cs;
	characterRecognition cr;
public:
	~Lprs();
	Lprs(string plateSvmTrainModel,string enTrainModel,string chTrainModel);
	string prosess(Mat image);      //返回 "1" 定位失败   "2"  分割失败
};
