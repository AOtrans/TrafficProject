#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/ml/ml.hpp>
#include "highgui.h"
#include "cv.h"
#include "cxcore.hpp"
#include "iostream"
#include "opencv2/opencv.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include <fstream>
#include <vector>
#include <math.h>
#include "ml.h"
#include <caffe/caffe.hpp>
#include "settingutil.h"
#include "DB/dbmanager.h"
extern SettingUtil util;
extern DBManager dbManager;
using namespace cv;
using namespace std;
using namespace caffe;
#endif // COMMON_H
