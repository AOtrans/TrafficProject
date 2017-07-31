#include "cartracker.h"
CarTracker* CarTracker::tracker=nullptr;

void drawRect(Mat &img,cv::Rect rect,const char *str="")
{
    cv::putText(img,str,cv::Point(rect.x-10,rect.y),1,2,Scalar(255,0,0));
    cv::rectangle(img,rect,Scalar(255,0,0));
}

CarTracker::CarTracker()
{
    shapeExtract=nullptr;
    colorExtract=nullptr;
    logoExtract=nullptr;
    plateExtract=nullptr;
    carDetector=nullptr;
}

CarTracker *CarTracker::getInstence()
{
    if(tracker==nullptr)
    {
        tracker=new CarTracker();
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
    if(carDetector!=nullptr)
        delete carDetector;
    if(plateExtract!=nullptr)
        delete plateExtract;
    if(tracker!=nullptr)
    {
        delete tracker;
        tracker=nullptr;
    }
}

std::vector<Prediction> CarTracker::getLogo(const cv::Mat &img, int top_k)
{
    std::cout<<"logoExtract init"<<std::endl;
    if(logoExtract==nullptr)
        logoExtract=new CarFeatureExtract("CarLogo");
    return logoExtract->singleImageCarFeatureExtract(img,top_k);
}

std::vector<Prediction> CarTracker::getShape(const cv::Mat &img, int top_k)
{
    std::cout<<"shapeExtract init"<<std::endl;
    if(shapeExtract==nullptr)
        shapeExtract=new CarFeatureExtract("CarShape");
    return shapeExtract->singleImageCarFeatureExtract(img,top_k);
}

std::vector<Prediction> CarTracker::getColor(const cv::Mat &img, int top_k)
{
    std::cout<<"colorExtract init"<<std::endl;
    if(colorExtract==nullptr)
        colorExtract=new CarFeatureExtract("CarColor");
    return colorExtract->singleImageCarFeatureExtract(img,top_k);
}

string CarTracker::getPlate(const Mat &img)
{
    if(plateExtract==nullptr)
    {
        std::cout<<"plateExtract init"<<std::endl;
        const char* tagName = "CarPlate";
        string plateSvmTrainModel = util.getValue(tagName,"PlateSvmTrainModel").toStdString();
        string enTrainModel = util.getValue(tagName,"EnTrainModel").toStdString();
        string chTrainModel = util.getValue(tagName,"ChTrainModel").toStdString();

        plateExtract=new Lprs(plateSvmTrainModel,enTrainModel,chTrainModel);
    }
    return plateExtract->prosess(img);
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
            auto dcolor = colorExtract->singleImageCarFeatureExtract(frame(*it).clone());
            auto dshape = shapeExtract->singleImageCarFeatureExtract(frame(*it).clone());
            if(compareColor(dcolor,color)
                    &&compareShape(dshape,shape))
            {
                isPointFrame=true;
                drawRect(frame,(*it));
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
    vector<Rect> results;
    if(carDetector==nullptr)
    {
        std::cout<<"detector init"<<std::endl;
        const char* tagName = "CarDetect";
        string model_file   = util.getValue(tagName,"modelFilePath").toStdString();
        string trained_file = util.getValue(tagName,"trainedFilePath").toStdString();
        string mean_file    = util.getValue(tagName,"meanFilePath").toStdString();
        string mean_value    = util.getValue(tagName,"meanValue").toStdString();
        confidenceThreshold   = util.getValue(tagName,"ConfidenceThreshold").toFloat();

        carDetector=new Detector(model_file, trained_file, mean_file, mean_value);
    }
    int pad=10;
    std::vector<vector<float> > detections = carDetector->Detect(img);
    for (int i = 0; i < detections.size(); ++i) {
        const vector<float>& d = detections[i];
        if (d[0] >= confidenceThreshold) {

            int x=(d[1]-pad>0)?(d[1]-pad):0;
            int y=(d[2]-pad>0)?(d[2]-pad):0;
            int width=(d[3]+pad<img.cols)?(d[3]-d[1]+pad):(img.cols-d[1]);
            int height=(d[4]+pad<img.rows)?(d[4]-d[2]+pad):(img.rows-d[2]);

            cv::Rect rect(x, y, width, height);
            results.push_back(rect);
        }
    }
    return results;
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
