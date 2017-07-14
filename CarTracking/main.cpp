#include "common.h"
typedef struct Location
{
    int x;
    int y;
    int width;
    int height;
}Location;

string getLogo(Mat img)
{

}

string getColor(Mat img)
{

}

string getShape(Mat img)
{

}

string getPlate(Mat img)
{

}

bool compareLogo(Mat img,string logo)
{
    return (logo==getLogo(img));
}

bool compareShape(Mat img,string shape)
{
    return (shape==getShape(img));
}

bool compareColor(Mat img,string color)
{
    return (color==getColor(img));
}

bool comparePlate(Mat img,string plate)
{
    return (plate==getPlate(img));
}

vector<pair<Mat,Location*> > getCars(Mat img)
{

}

void drawRect(Mat &img,Location *location)
{
    cv::rectangle(img,Rect(location->x,location->y,location->width,location->height),Scalar(255,0,0));
}

string carTrack(string fileName,string logo,string shape,string color,string plate)
{
    cv::VideoCapture capture(fileName);
    if (!capture.isOpened())
    {
        cout<<"capture not opened"<<endl;
        return "";
    }

    string outputVideoName = "./output_lkj_local.avi";
    VideoWriter writer;

    int frameRate = 25;
    Size frameSize;
    if (!writer.isOpened())
    {
        frameSize.width  = capture.get(CV_CAP_PROP_FRAME_WIDTH);;
        frameSize.height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);;

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
        vector<pair<Mat,Location*> > &&cars=getCars(frame);
        for(vector<pair<Mat,Location*> >::iterator it=cars.begin();it!=cars.end();it++)
        {
            if(compareColor((*it).first,color)
                    &&compareShape((*it).first,shape)
                    &&compareLogo((*it).first,logo)
                    &&comparePlate((*it).first,plate))
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

int main(int argc, char *argv[])
{
    Mat img=imread("");
    cout<<carTrack("D:/work/ppt/xx.mp4",getLogo(img),getShape(img),getColor(img),getPlate(img))<<endl;
    return 0;
}
