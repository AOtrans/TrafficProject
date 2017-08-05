#ifndef CARFEATUREEXTRACT_H
#define CARFEATUREEXTRACT_H

#define CPU_ONLY//remove if you have cuda configure
#include "common.h"
#include "classifier.h"

class CarFeatureExtract
{
public:
    CarFeatureExtract(const char *tagName);

    CarFeatureExtract();

    ~CarFeatureExtract();

    bool reInit(const char *tagName);

    vector<std::vector<Prediction> > imagesCarFeatureExtract(const vector<cv::Mat> &images, int top_k=5); //predict a group of Mats

    vector<std::vector<Prediction> > imagePathsCarFeatureExtract(const vector<const char*> &imageFilePaths, int top_k=5);//predict a group of Mats dependence on Paths

    std::vector<Prediction> singleImagePathCarFeatureExtract(const char* imageFilePath, int top_k=5);//predict single Mat, auto read from giving filePath

    std::vector<Prediction> singleImageCarFeatureExtract(cv::Mat img, int top_k=5);//predict single Mat
    //bool checkImageFormat(cv::Mat &img); //check and format image
private:
    Classifier* classifier;
};


#endif // CARFEATUREEXTRACT_H
