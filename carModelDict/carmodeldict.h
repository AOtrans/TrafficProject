#ifndef CARMODELDICT_H
#define CARMODELDICT_H

#define CPU_ONLY//remove if you have cuda configure

#include "iniutil.h"
#include <caffe/caffe.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <algorithm>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>
#include <vector>
class Classifier;


class CarModelDict
{
public:
    CarModelDict(const char* configFilePath);
    CarModelDict();
    ~CarModelDict();
    bool reInit(const char* configFilePath);
    vector<string> singleImagesCarModelDict(const vector<cv::Mat> &images); //predict a group of Mats
    vector<string> singleImagePathsCarModelDict(const vector<const char*> &imageFilePaths);//predict a group of Mats dependence on Paths
    string singleImagePathCarModelDict(const char* imageFilePath);//predict single Mat, auto read from giving filePath
    string singleImageCarModelDict(cv::Mat img);//predict single Mat
    bool checkImageFormat(cv::Mat &img); //check and format image
private:
    Classifier* classifier;

};


#endif // CARMODELDICT_H
