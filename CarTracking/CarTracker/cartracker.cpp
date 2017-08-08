#include "cartracker.h"
CarTracker* CarTracker::tracker=nullptr;

void drawRect(Mat &img,cv::Rect rect,const char *str="")
{
    cv::putText(img,str,cv::Point(rect.x-10,rect.y),1,2,Scalar(255,0,0));
    cv::rectangle(img,rect,Scalar(255,0,0));
}

CarTracker::CarTracker(const char *configFilePath):configFilePath(configFilePath)
{
    shapeExtract=nullptr;
    colorExtract=nullptr;
    logoExtract=nullptr;
}

CarTracker *CarTracker::getInstence(const char *configFilePath)
{
    if(tracker==nullptr&&configFilePath!="")
    {
        tracker=new CarTracker(configFilePath);
    }
    return tracker;
}

CarTracker::~CarTracker()
{
    if(shapeExtract!=nullptr)
        delete shapeExtract;
    if(colorExtract!=nullptr)
        delete colorExtract;
    if(logoExtract!=nullptr)
        delete logoExtract;
    if(tracker!=nullptr)
    {
        delete tracker;
        tracker=nullptr;
    }
}

std::vector<Prediction> CarTracker::getLogo(const cv::Mat &img, int top_k)
{
    if(logoExtract==nullptr)
        logoExtract=new CarFeatureExtract(configFilePath,"CarLogo");
    return logoExtract->singleImageCarFeatureExtract(img,top_k);
}

std::vector<Prediction> CarTracker::getShape(const cv::Mat &img, int top_k)
{
    if(shapeExtract==nullptr)
        shapeExtract=new CarFeatureExtract(configFilePath,"CarShape");
    return shapeExtract->singleImageCarFeatureExtract(img,top_k);
}

std::vector<Prediction> CarTracker::getColor(const cv::Mat &img, int top_k)
{
    if(colorExtract==nullptr)
        colorExtract=new CarFeatureExtract(configFilePath,"CarColor");
    return colorExtract->singleImageCarFeatureExtract(img,top_k);
}

string CarTracker::getPlate(const Mat &img)
{

}

float *CarTracker::getSiftFeature(Mat &img)
{
    SiftDescriptorExtractor extractor;
        vector<KeyPoint> keypoints;
        cv::Mat des;
        float *data=new float[128];
        float *temp=data;
        int step = 10; // 10 pixels spacing between kp's

        for (int y=step; y<src.rows-step; y+=step){
            for (int x=step; x<src.cols-step; x+=step){
                // x,y,radius
                keypoints.push_back(KeyPoint(float(x), float(y), float(step)));
            }
        }
        extractor.compute(img, keypoints, des);

        for(int i=0;i<des.cols;i++)
        {
            float max=0.0;
            for(int j=0;j<des.rows;j++)
            {
                max=fabs(pcaDes.at<float>(j,i))>max?fabs(pcaDes.at<float>(j,i)):max;
            }
            *(temp++)=max;
        }

        return data;
}

string CarTracker::carTrack(string videoFileName, string shape, string color, string logo, string plate)
{
    cv::VideoCapture capture(videoFileName);
    if (!capture.isOpened())
    {
        cout<<"capture not opened"<<endl;
        return "";
    }

    string outputVideoName = "./output_local.avi";
    VideoWriter writer;

    int frameRate = 25;
    Size frameSize;
    if (!writer.isOpened())
    {
        frameSize.width  = capture.get(CV_CAP_PROP_FRAME_WIDTH);
        frameSize.height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);

        if (!writer.open(outputVideoName, CV_FOURCC('D','I','V','X') , frameRate, frameSize, true))
        {
            cout << "open writer error..." << endl;
            return "";
        }
    }

    double rate= capture.get(CV_CAP_PROP_FPS);
    cv::Mat frame;
    cv::namedWindow("Extracted Frame");
    int delay= rate;
    while (capture.read(frame))
    {
        bool isPointFrame=false;
        vector<cv::Rect> &&cars=getCars(frame);
        for(vector<cv::Rect>::iterator it=cars.begin();it!=cars.end();it++)
        {

            if(compareColor((*it).first,color)
                    &&compareShape((*it).first,shape))
            {
                isPointFrame=true;
                drawRect(frame,(*it).second);
            }
        }
        cv::imshow("Extracted Frame",frame);
        if(isPointFrame)
        {
            writer.write(frame);
        }
        if (cv::waitKey(delay)>=0)
            break;
    }
    writer.release();
    capture.release();
    return outputVideoName;
}

vector<Rect> CarTracker::getCars(Mat &img)
{

}

bool CarTracker::compareCOSLike(float *t1, float *t2, int count)
{
    float dotSum=0.0f;
    float modeT1=0.0f,modeT2=0.0f;
    for(int i=0;i<count;i++)
    {
        dotSum+=t1[i]*t2[i];
        modeT1+=t1[i]*t1[i];
        modeT2+=t2[i]*t2[i];
    }
    if(modeT1==0||modeT2==0)
        return false;
    else
        return (acos(dotSum/(sqrt(fabs(modeT1))*sqrt(fabs(modeT2))))<=0.1);
}

bool CarTracker::compareShape(std::vector<Prediction> &result,string shape)
{
    for(int i=0;i<2;i++)
    {
        if(result.at(i).first==shape)
            return true;
        if(result.at(i).second>0.5)
            break;
    }
    return false;
}

bool CarTracker::compareColor(std::vector<Prediction> &result,string color)
{
    for(int i=0;i<2;i++)
    {
        if(result.at(i).first==color)
            return true;
        if(result.at(i).second>0.5)
            break;
    }
    return false;
}
