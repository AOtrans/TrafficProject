#include "cartracker.h"

CarTracker* CarTracker::tracker = nullptr;

void drawRect(Mat &img, cv::Rect rect, const char *str = "")
{
    cv::putText(img, str, cv::Point(rect.x-10,rect.y), 1, 2, Scalar(255,0,0));
    cv::rectangle(img, rect, Scalar(255,0,0));
}

CarTracker::CarTracker()
{
    videoSavePath = util.getValue("urlConfig", "VideoSavePath");
    imageSavePath = util.getValue("urlConfig", "ImageSavePath");

    shapeExtract = nullptr;
    colorExtract = nullptr;
    logoExtract = nullptr;
    plateExtract = nullptr;
    carDetector = nullptr;
    truckDetector = nullptr;
    motoShapeExtract = nullptr;
    motoDetector = nullptr;
    trainfo = nullptr;
}

CarTracker *CarTracker::getInstence()
{
    if(tracker == nullptr)
    {
        tracker = new CarTracker();
    }
    return tracker;
}

CarTracker::~CarTracker()
{
    if(shapeExtract != nullptr)
        delete shapeExtract;
    if(colorExtract != nullptr)
        delete colorExtract;
    if(logoExtract != nullptr)
        delete logoExtract;
    if(carDetector != nullptr)
        delete carDetector;
    if(plateExtract != nullptr)
        delete plateExtract;
    if(truckDetector != nullptr)
        delete truckDetector;
    if(motoShapeExtract != nullptr)
        delete motoShapeExtract;
    if(motoDetector != nullptr)
        delete motoDetector;
    if(trainfo != nullptr)
        delete trainfo;

    if(tracker != nullptr)
    {
        delete tracker;
        tracker = nullptr;
    }
}

std::vector<Prediction> CarTracker::getLogo(const cv::Mat &img, int top_k)
{
    qDebug() << "logoExtract init";

    if(logoExtract == nullptr)
        logoExtract = new CarFeatureExtract("CarLogo");

    return logoExtract->singleImageCarFeatureExtract(img, top_k);
}

std::vector<Prediction> CarTracker::getShape(const cv::Mat &img, int top_k)
{
    qDebug() << "shapeExtract init";

    if(shapeExtract == nullptr)
        shapeExtract = new CarFeatureExtract("CarShape");

    return shapeExtract->singleImageCarFeatureExtract(img, top_k);
}

std::vector<Prediction> CarTracker::getColor(const cv::Mat &img, int top_k)
{
    qDebug() << "colorExtract init";

    if(colorExtract == nullptr)
        colorExtract = new CarFeatureExtract("CarColor");

    return colorExtract->singleImageCarFeatureExtract(img, top_k);
}

std::vector<Prediction> CarTracker::getMotoShape(const Mat &img, int top_k)
{
    qDebug() << "motoShapeExtract init";

    if(motoShapeExtract == nullptr)
        motoShapeExtract = new CarFeatureExtract("MotoShape");

    return motoShapeExtract->singleImageCarFeatureExtract(img, top_k);
}

string CarTracker::getPlate(const Mat &img)
{
    if(plateExtract == nullptr)
    {
        qDebug() << "plateExtract init";

        const char *tagName = "CarPlate";
        string plateSvmTrainModel = util.getValue(tagName, "PlateSvmTrainModel").toStdString();
        string enTrainModel = util.getValue(tagName, "EnTrainModel").toStdString();
        string chTrainModel = util.getValue(tagName, "ChTrainModel").toStdString();

        plateExtract = new Lprs(plateSvmTrainModel, enTrainModel, chTrainModel);
    }
    return plateExtract->prosess(img);
}

float* CarTracker::getSiftFeature(Mat img)
{
    SiftDescriptorExtractor extractor;
    vector<KeyPoint> keypoints;
    cv::Mat des;
    float *data = new float[128];
    float *temp = data;
    int step = 10; // 10 pixels spacing between kp's

    for (int y = step; y < img.rows-step; y += step){
        for (int x = step; x < img.cols-step; x += step){
            // x,y,radius
            keypoints.push_back(KeyPoint(float(x), float(y), float(step)));
        }
    }

    extractor.compute(img, keypoints, des);

    for(int i = 0; i < des.cols; i++)
    {
        float max = 0.0;
        for(int j = 0; j < des.rows; j++)
        {
            max=fabs(des.at<float>(j,i)) > max?fabs(des.at<float>(j,i)):max;
        }
        *(temp++) = max;
    }

    return data;
}

void CarTracker::removeUselessCarHistCache(vector<CarHistCache> &chc)
{
    int size = chc.size();
    for (int i = 0; i < size; i++)
    {
        if (!chc[i].isUpdate && chc[i].positionChange)
        {
            chc.erase(chc.begin() + i);
            i--;
            size--;
        }
    }
}

void CarTracker::getHist(Mat image, float hist[], int count)
{
    if (image.channels() > 1)
        cvtColor(image, image, COLOR_BGR2GRAY);

    int histSize[] = { 256 };
    float granges[] = { 0, 256 };
    const float* ranges[] = { granges };
    MatND histND;
    int channels[] = { 0 };

    calcHist(&image, 1, channels, Mat(), histND, 1, histSize, ranges, true, false);

    for (int i = 0; i < count; i++)
        hist[i] = histND.at<float>(i, 0);
}


void CarTracker::initialCarHistCache(int dealType,string shape, string color, cv::VideoCapture &capture, vector<CarHistCache> &chc)
{
    Mat frame;

    while (capture.read(frame))
    {
        bool isPointFrame = false;
        vector<cv::Rect> &&cars = getCars(frame);

        for (vector<cv::Rect>::iterator it = cars.begin(); it != cars.end(); it++)
        {
            Mat dealCar = frame((*it)).clone();

            if (dealType == 1)
            {
                auto dcolor = getColor(frame(*it).clone(), 2);
                auto dshape = getShape(frame(*it).clone(), 2);

                if (compareColor(dcolor, color)
                    && compareShape(dshape, shape))
                {
                    CarHistCache subChc;
                    getHist(dealCar, subChc.backFrameCarHist, 256);
                    subChc.backFrameCarposition = (*it);
                    subChc.isUpdate = true;
                    subChc.num = 1;
                    subChc.positionChange = false;
                    chc.push_back(subChc);

                    isPointFrame = true;
                    drawRect(frame, (*it));
                }
            }
            else if (dealType == 2)
            {
                auto dshape = shapeExtract->singleImageCarFeatureExtract(frame(*it).clone());

                if (compareShape(dshape, shape))
                {
                    CarHistCache subChc;
                    getHist(dealCar, subChc.backFrameCarHist, 256);
                    subChc.backFrameCarposition = (*it);
                    subChc.isUpdate = true;
                    subChc.num = 1;
                    subChc.positionChange = false;
                    chc.push_back(subChc);

                    isPointFrame = true;
                    drawRect(frame, (*it));
                }
            }
            else if (dealType == 3)
            {
                auto dcolor = colorExtract->singleImageCarFeatureExtract(frame(*it).clone());

                if (compareColor(dcolor, color))
                {
                    CarHistCache subChc;
                    getHist(dealCar, subChc.backFrameCarHist, 256);
                    subChc.backFrameCarposition = (*it);
                    subChc.isUpdate = true;
                    subChc.num = 1;
                    subChc.positionChange = false;
                    chc.push_back(subChc);

                    isPointFrame = true;
                    drawRect(frame, (*it));
                }
            }
        }

        //cv::imshow("initialCarHistCache Extracted Frame", frame);

        if (isPointFrame)
        {
            break;
        }
    }
}

string CarTracker::carTrack(string videoFileName, string shape, string color, string logo, string plate)
{
    vector<CarHistCache> chc;
    cv::VideoCapture capture(videoFileName);
    capture.open(videoFileName);

    if (!capture.isOpened())
    {
        qDebug() << "capture not opened";
        return "";
    }

    string outputVideoName = videoSavePath.toStdString() + "/" + QUuid::createUuid().toString().replace("}", "").replace("{", "").toStdString() + ".avi";
    VideoWriter writer;

    int frameRate = 25;
    Size frameSize;

    if (!writer.isOpened())
    {
        frameSize.width  = capture.get(CV_CAP_PROP_FRAME_WIDTH);
        frameSize.height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);

        if (!writer.open(outputVideoName, CV_FOURCC('D','I','V','X') , frameRate, frameSize, true))
        {
            qDebug() << "open writer error...";
            return "";
        }
    }

    double rate = capture.get(CV_CAP_PROP_FPS);
    cv::Mat frame;
    cv::namedWindow("Extracted Frame");
    int delay = rate;

    //根据给的车型，颜色参数，判断根据车型、颜色的组合筛选
    int dealType;           // 为1 车型、颜色 2 车型 3颜色
    if (!shape.empty() && !color.empty())
        dealType = 1;
    else if (!shape.empty())
        dealType = 2;
    else if (!color.empty())
        dealType = 3;
    else
    {
        qDebug() << "function parameter shape and color have no data ...";
        return "";
    }

    float currentFrameHist[256];  //当前帧的灰度直方图

    initialCarHistCache(dealType,shape, color, capture, chc);

    while (capture.read(frame))
    {
        //imshow("readImage",frame);
        //waitKey(1);

        for (int i = 0; i < chc.size(); i++)
            chc[i].isUpdate = false;

        vector<cv::Rect> &&cars = getCars(frame);

        for (vector<cv::Rect>::iterator it = cars.begin(); it != cars.end(); it++)
        {
            Mat dealCar = frame((*it)).clone();
            int left = (*it).x;
            int right = (*it).x + (*it).width;
            int down = (*it).y;
            int up = (*it).y + (*it).height;

            getHist(dealCar, currentFrameHist, 256);
            bool isInChc = false;

            if (dealType == 1)
            {
                auto dcolor = getColor(frame(*it).clone(), 2);
                auto dshape = getShape(frame(*it).clone(), 2);

                if (compareColor(dcolor, color) && compareShape(dshape, shape))
                {
                    for (int j = 0; j < chc.size(); j++)
                    {
                        if (compareCOSLike(chc[j].backFrameCarHist, currentFrameHist, 256))
                        {
                            chc[j].isUpdate = true;
                            chc[j].num++;
                            isInChc = true;

                            //当车位置不重合时，判断车移动了
                            int backLeft = chc[j].backFrameCarposition.x;
                            int backRight = chc[j].backFrameCarposition.x + chc[j].backFrameCarposition.width;
                            int backDown = chc[j].backFrameCarposition.y;
                            int backUp = chc[j].backFrameCarposition.y + chc[j].backFrameCarposition.height;

                            bool nonOverLap = left > backRight || right < backLeft || down > backUp || up < backDown;
                            if (nonOverLap)
                                chc[j].positionChange = true;
                        }
                    }

                    if (!isInChc)
                    {
                        CarHistCache subChc;
                        memcpy(subChc.backFrameCarHist, currentFrameHist, sizeof(currentFrameHist));
                        subChc.backFrameCarposition = (*it);
                        subChc.isUpdate = true;
                        subChc.num = 1;
                        subChc.positionChange = false;
                        chc.push_back(subChc);
                    }

                    drawRect(frame, (*it));
                }
            }
            else if (dealType == 2)
            {
                auto dshape = getShape(frame(*it).clone(), 2);

                if (compareShape(dshape, shape))
                {
                    for (int j = 0; j < chc.size(); j++)
                    {
                        if (compareCOSLike(chc[j].backFrameCarHist, currentFrameHist, 256))
                        {
                            chc[j].isUpdate = true;
                            chc[j].num++;
                            isInChc = true;

                            //当车位置不重合时，判断车移动了
                            int backLeft = chc[j].backFrameCarposition.x;
                            int backRight = chc[j].backFrameCarposition.x + chc[j].backFrameCarposition.width;
                            int backDown = chc[j].backFrameCarposition.y;
                            int backUp = chc[j].backFrameCarposition.y + chc[j].backFrameCarposition.height;

                            bool nonOverLap = left > backRight || right < backLeft || down > backUp || up < backDown;
                            if (nonOverLap)
                                chc[j].positionChange = true;
                        }
                    }

                    if (!isInChc)
                    {
                        CarHistCache subChc;
                        memcpy(subChc.backFrameCarHist, currentFrameHist, sizeof(currentFrameHist));
                        subChc.backFrameCarposition = (*it);
                        subChc.isUpdate = true;
                        subChc.num = 1;
                        subChc.positionChange = false;
                        chc.push_back(subChc);
                    }

                    drawRect(frame, (*it));
                }
            }
            else if (dealType == 3)
            {
                auto dcolor = getColor(frame(*it).clone(), 2);

                if (compareColor(dcolor, color))
                {
                    for (int j = 0; j < chc.size(); j++)
                    {
                        if (compareCOSLike(chc[j].backFrameCarHist, currentFrameHist, 256))
                        {
                            chc[j].isUpdate = true;
                            chc[j].num++;
                            isInChc = true;

                            //当车位置不重合时，判断车移动了
                            int backLeft = chc[j].backFrameCarposition.x;
                            int backRight = chc[j].backFrameCarposition.x + chc[j].backFrameCarposition.width;
                            int backDown = chc[j].backFrameCarposition.y;
                            int backUp = chc[j].backFrameCarposition.y + chc[j].backFrameCarposition.height;

                            bool nonOverLap = left > backRight || right < backLeft || down > backUp || up < backDown;
                            if (nonOverLap)
                                chc[j].positionChange = true;
                        }
                    }

                    if (!isInChc)
                    {
                        CarHistCache subChc;
                        memcpy(subChc.backFrameCarHist, currentFrameHist, sizeof(currentFrameHist));
                        subChc.backFrameCarposition = (*it);
                        subChc.isUpdate = true;
                        subChc.num = 1;
                        subChc.positionChange = false;
                        chc.push_back(subChc);
                    }

                    drawRect(frame, (*it));
                }
            }
        }

        bool isWrite = false;
        for (int i = 0; i < chc.size(); i++)
        {
            if (chc[i].isUpdate && chc[i].num < 13)
            {
                isWrite = true;
                break;
            }
        }

        cv::imshow("Extracted Frame", frame);

        if (isWrite)
        {
            imshow("wirteImage",frame);
            waitKey(1);
            writer.write(frame);
        }

        removeUselessCarHistCache(chc);

        if (cv::waitKey(delay) >= 0)
            break;

    }

    writer.release();
    capture.release();

    return outputVideoName;
}

string CarTracker::motoTrack(string videoFileName, string shape)
{
    cv::VideoCapture capture(videoFileName);

    if (!capture.isOpened())
    {
        qDebug() << "capture not opened";
        return "";
    }

    string outputVideoName = videoSavePath.toStdString() + "/" + "output_local.avi";
    VideoWriter writer;

    int frameRate = 25;
    Size frameSize;

    if (!writer.isOpened())
    {
        frameSize.width  = capture.get(CV_CAP_PROP_FRAME_WIDTH);
        frameSize.height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);

        if (!writer.open(outputVideoName, CV_FOURCC('D','I','V','X') , frameRate, frameSize, true))
        {
            qDebug() << "open writer error...";
            return "";
        }
    }

    double rate = capture.get(CV_CAP_PROP_FPS);
    cv::Mat frame;
    cv::namedWindow("Extracted Frame");
    int delay = rate;

    while (capture.read(frame))
    {
        bool isPointFrame = false;
        vector<cv::Rect> &&motos = getMotos(frame);

        for(vector<cv::Rect>::iterator it = motos.begin(); it != motos.end(); it++)
        {
            auto dshape = getMotoShape(frame(*it).clone(), 2);

            if(compareShape(dshape, shape))
            {
                isPointFrame = true;
                drawRect(frame, (*it));
            }
        }

        cv::imshow("Extracted Frame", frame);

        if(isPointFrame)
        {
            writer.write(frame);
        }

        if (cv::waitKey(delay) >= 0)
            break;

    }

    writer.release();
    capture.release();

    return outputVideoName;
}

void CarTracker::truckTrack(string videoFileName, string startTime, string channelCode)
{
    cv::VideoCapture capture(videoFileName);
    QVector<CarInfo *> matchList;

    if (!capture.isOpened())
    {
        qDebug() << "capture not opened";
        return;
    }

    float rate = capture.get(CV_CAP_PROP_FPS);
    float usPerFrame = 1000/rate;

    cv::Mat frame;
    QDateTime time = QDateTime::fromTime_t(atoi(startTime.c_str()));
    int cross = 3;
    int num = 0;

    while (capture.read(frame))
    {
        time = time.addMSecs(usPerFrame);

        if((num++)%cross != 0)
        {
            continue;
        }

        getTrucks(frame, QString::number(time.toTime_t()), QString(channelCode.c_str()), matchList);

        excuteMatchList(matchList);
    }

    for(int i = 0; i < matchList.size(); i++)
        delete matchList.at(i);

    capture.release();
}

void CarTracker::taxiTrack(string videoFileName, string startTime, string channelCode)
{
    cv::VideoCapture capture(videoFileName);
    QVector<CarInfo *> matchList;

    if (!capture.isOpened())
    {
        qDebug() << "capture not opened";
        return;
    }

    float rate = capture.get(CV_CAP_PROP_FPS);
    float usPerFrame = 1000/rate;
    qDebug() << "uspf:" << usPerFrame;

    cv::Mat frame;
    QDateTime time = QDateTime::fromTime_t(atoi(startTime.c_str()));
    int cross = 3;
    int num = 0;

    while (capture.read(frame))
    {
        time = time.addMSecs(usPerFrame);

        if((num++)%cross != 0)
        {
            continue;
        }

        vector<cv::Rect> &&cars = getCars(frame);
        bool isPointFrame = false;

        for(vector<cv::Rect>::iterator it = cars.begin(); it != cars.end(); it++)
        {
            if(!checkMatchList(frame(*it), matchList) )
            {
                cv::Mat mat = frame(*it).clone();

                int width = (*it).width;
                int height = (*it).height;

                if(width > height)
                    cv::copyMakeBorder(mat, mat, (width - height)/2,  (width - height)/2, 0, 0, cv::BORDER_CONSTANT, Scalar(0,0,0) );
                else if(height > width)
                    cv::copyMakeBorder(mat, mat, 0, 0, (height - width)/2,  (height - width)/2, cv::BORDER_CONSTANT, Scalar(0,0,0) );

                auto dcolor = getColor(mat, 2);
                auto dshape = getShape(mat, 2);

                if(dcolor.at(0).first == "green" && (dshape.at(0).first == "sedan" || dshape.at(0).first == "hatchback"))
                {
                    cv::rectangle(frame, *it, Scalar(0, 0, 255), 2);
                    cv::putText(frame,dcolor.at(0).first+"_"+dshape.at(0).first,cv::Point((*it).x-5,(*it).y),0,1,Scalar(0,0,255),2);
                    isPointFrame = true;
                }

                CarInfo *info = new CarInfo;
                info->boundingBox = *it;
                info->matchNums = 0;
                info->feature = getHistFeature(frame(*it));
                info->mat = frame(*it).clone();

                matchList.push_back(info);

                cv::imshow("not match",frame(*it));
                cv::waitKey();
            }
            else
            {
                qDebug() << "------------------------match---------------------";
            }
        }

        excuteMatchList(matchList);

        if(isPointFrame)
        {
            QString imageUrl = imageSavePath + "/" + QUuid::createUuid().toString().replace("{", "").replace("}", "") + ".jpg";
            cv::imwrite(imageUrl.toStdString(), frame);
            QString sql = "insert into carPic_info(channel_id,carPic_time,carPic_url,carPic_type) values('%1','%2','%3','%4')";

            dbManager.execQuery(sql.arg(QString(channelCode.c_str()) ).arg(QString::number(time.toTime_t()) ).arg(imageUrl).arg("taxi"));
        }
    }

    for(int i = 0; i < matchList.size(); i++)
        delete matchList.at(i);

    capture.release();
}

void CarTracker::areaCarTrack(string videoFileName, string startTime, string channelCode, string areas)
{
    QStringList t = QString(areas.c_str()).split(",");
    cout<< t.size() <<endl;
    vector<std::pair<cv::Point2f,cv::Point2f> > rectAreas;

    for(int i = 0; i < t.size()/4; i++)
    {
        rectAreas.push_back(getPoints(t.at(i*4).toFloat(),
                                      t.at(i*4+1).toFloat(),
                                      t.at(i*4+2).toFloat(),
                                      t.at(i*4+3).toFloat()));
    }

    cv::VideoCapture capture(videoFileName);

    if (!capture.isOpened())
    {
        qDebug() << "capture not opened";
        return;
    }

    float rate = capture.get(CV_CAP_PROP_FPS);
    float usPerFrame = 1000/rate;

    cv::Mat frame;
    QDateTime time = QDateTime::fromTime_t(atoi(startTime.c_str()));
    int cross = 20;
    int num = 0;

    while (capture.read(frame))
    {
        time = time.addMSecs(usPerFrame);
        bool isPointFrame = false;

        if((num++)%cross != 0)
        {
            continue;
        }

        for(int i = 0; i < rectAreas.size(); i++)
        {
            int tx1 = rectAreas.at(i).first.x*frame.cols;
            int ty1 = rectAreas.at(i).first.y*frame.rows;
            int tx2 = rectAreas.at(i).second.x*frame.cols;
            int ty2 = rectAreas.at(i).second.y*frame.rows;

            cv::Rect tRect(tx1, ty1, tx2-tx1, ty2-ty1);
            cv::rectangle(frame, tRect, Scalar(255, 0, 0), 2);
        }

        vector<cv::Rect> &&cars = getCars(frame);

        for(vector<cv::Rect>::iterator it = cars.begin(); it != cars.end(); it++)
        {
            if(inRect(*it,rectAreas, frame.cols, frame.rows))
            {
                cv::rectangle(frame, *it, Scalar(0, 0, 255), 2);
                isPointFrame = true;
            }
        }

        if(isPointFrame)
        {
            QString imageUrl = imageSavePath + "/" + QUuid::createUuid().toString().replace("{", "").replace("}", "") + ".jpg";
            cv::imwrite(imageUrl.toStdString(),frame);
            QString sql = "insert into carPic_info(channel_id,carPic_time,carPic_url,carPic_type) values('%1','%2','%3','%4')";

            dbManager.execQuery(sql.arg(QString(channelCode.c_str()) ).arg(QString::number(time.toTime_t()) ).arg(imageUrl).arg("areacar"));
        }
    }

    capture.release();
}

string CarTracker::trafficStatistics(string videoFileName, string areas)
{
    if(carDetector == nullptr)
    {
        qDebug() << "car detector init";

        const char* tagName = "CarDetect";
        string model_file   = util.getValue(tagName, "modelFilePath").toStdString();
        string trained_file = util.getValue(tagName, "trainedFilePath").toStdString();
        string mean_file    = util.getValue(tagName, "meanFilePath").toStdString();
        string mean_value   = util.getValue(tagName, "meanValue").toStdString();
        carThreshold = util.getValue(tagName, "ConfidenceThreshold").toFloat();

        carDetector = new Detector(model_file, trained_file, mean_file, mean_value);
    }

    if(trainfo == nullptr)
    {
        trainfo = new Trainfo();
    }

    cv::VideoCapture capture(videoFileName);

    if (!capture.isOpened())
    {
        qDebug() << "capture not opened";
        return "";
    }

    int width  = capture.get(CV_CAP_PROP_FRAME_WIDTH);
    int height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);


    QStringList t = QString(areas.c_str()).split(",");

    std::vector<vector<float> > rectboxes;

    for(int i = 0; i < t.size()/4; i++)
    {
        auto points = getPoints(t.at(i*4).toFloat(),
                                t.at(i*4+1).toFloat(),
                                t.at(i*4+2).toFloat(),
                                t.at(i*4+3).toFloat());
        cv::Point2f p1 = points.first;
        cv::Point2f p2 = points.second;

        std::vector<float> rectboxtrans;
        rectboxtrans.push_back(0);
        rectboxtrans.push_back(p1.x*width);
        rectboxtrans.push_back(p1.y*height);
        rectboxtrans.push_back(p2.x*width);
        rectboxtrans.push_back(p2.y*height);

        rectboxes.push_back(rectboxtrans);
    }

    //    std::vector<float> rectboxtrans;
    //    rectboxtrans.push_back(0);
    //    rectboxtrans.push_back(800);
    //    rectboxtrans.push_back(500);
    //    rectboxtrans.push_back(800 + 260);
    //    rectboxtrans.push_back(500 + 200);

    //    std::vector<float> rectboxtrans_1;
    //    rectboxtrans_1.push_back(0);
    //    rectboxtrans_1.push_back(100);
    //    rectboxtrans_1.push_back(500);
    //    rectboxtrans_1.push_back(100 + 260);
    //    rectboxtrans_1.push_back(500 + 200);

    //    std::vector<float> rectboxtrans_2;
    //    rectboxtrans_2.push_back(0);
    //    rectboxtrans_2.push_back(460);
    //    rectboxtrans_2.push_back(500);
    //    rectboxtrans_2.push_back(460 + 260);
    //    rectboxtrans_2.push_back(500 + 200);

    //    std::vector<vector<float> > rectboxes;
    //    rectboxes.push_back(rectboxtrans);
    //    rectboxes.push_back(rectboxtrans_1);
    //    rectboxes.push_back(rectboxtrans_2);

    string outputVideoName = videoSavePath.toStdString() + "/" + "output_local.avi";
    //路口画框
    trainfo->CrossingDemo(carDetector, videoFileName, outputVideoName, rectboxes);

    return outputVideoName;
}

pair<Point2f, Point2f> CarTracker::getPoints(float a1, float a2, float b1, float b2)
{
    float temp;

    if(a1 > b1)
    {
        temp = a1;
        a1 = b1;
        b1 = temp;
    }

    if(a2 > b2)
    {
        temp = a2;
        a2 = b2;
        b2 = temp;
    }


    std::cout << a1 << " " << a2 << "," << b1 << " " << b2 << "**************************************" <<endl;
    return std::make_pair(cv::Point2f(a1, a2), cv::Point2f(b1, b2));
}

bool CarTracker::inRect(Rect rect, vector<pair<Point2f, Point2f> > rectAreas, int cols, int rows)
{
    for(int i = 0; i < rectAreas.size(); i++)
    {
        int tx1 = rectAreas.at(i).first.x*cols;
        int ty1 = rectAreas.at(i).first.y*rows;
        int tx2 = rectAreas.at(i).second.x*cols;
        int ty2 = rectAreas.at(i).second.y*rows;
        cv::Rect tRect(tx1, ty1, tx2-tx1, ty2-ty1);

        int x0 = std::max(rect.x, tRect.x);
        int x1 = std::min(rect.x + rect.width, tRect.x + tRect.width);
        int y0 = std::max(rect.y, tRect.y);
        int y1 = std::min(rect.y + rect.height, tRect.y + tRect.height);

        if ((x0 < x1) && (y0 < y1))
            return true;
    }

    return false;
}

QString CarTracker::getImageSavePath()
{
    return imageSavePath;
}

vector<Rect> CarTracker::getCars(Mat &img)
{
    vector<Rect> results;

    if(carDetector == nullptr)
    {
        qDebug() << "car detector init";

        const char* tagName = "CarDetect";
        string model_file   = util.getValue(tagName, "modelFilePath").toStdString();
        string trained_file = util.getValue(tagName, "trainedFilePath").toStdString();
        string mean_file    = util.getValue(tagName, "meanFilePath").toStdString();
        string mean_value   = util.getValue(tagName, "meanValue").toStdString();
        carThreshold = util.getValue(tagName, "ConfidenceThreshold").toFloat();

        carDetector = new Detector(model_file, trained_file, mean_file, mean_value);
    }

    int pad=10;
    std::vector<vector<float> > detections = carDetector->Detect(img);

    for (int i = 0; i < detections.size(); ++i) {
        const vector<float> &d = detections[i];
        if (d[0] >= carThreshold) {

            int x=(d[1]-pad > 0)?(d[1]-pad):0;
            int y=(d[2]-pad > 0)?(d[2]-pad):0;
            int width=(d[3]+pad < img.cols)?(d[3]-d[1]+pad):(img.cols-d[1]);
            int height=(d[4]+pad < img.rows)?(d[4]-d[2]+pad):(img.rows-d[2]);

            cv::Rect rect(x, y, width, height);
            results.push_back(rect);
        }
    }

    return results;
}

vector<Rect> CarTracker::getMotos(Mat &img)
{
    vector<Rect> results;

    if(motoDetector == nullptr)
    {
        qDebug() << "moto detector init";

        const char* tagName = "MotoDetect";
        string model_file   = util.getValue(tagName, "modelFilePath").toStdString();
        string trained_file = util.getValue(tagName, "trainedFilePath").toStdString();
        string mean_file    = util.getValue(tagName, "meanFilePath").toStdString();
        string mean_value   = util.getValue(tagName, "meanValue").toStdString();
        motoThreshold = util.getValue(tagName, "ConfidenceThreshold").toFloat();

        motoDetector = new Detector(model_file, trained_file, mean_file, mean_value);
    }

    int pad=10;
    std::vector<vector<float> > detections = motoDetector->Detect(img);

    for (int i = 0; i < detections.size(); ++i) {
        const vector<float> &d = detections[i];
        if (d[0] >= motoThreshold) {

            int x=(d[1]-pad > 0)?(d[1]-pad):0;
            int y=(d[2]-pad > 0)?(d[2]-pad):0;
            int width=(d[3]+pad < img.cols)?(d[3]-d[1]+pad):(img.cols-d[1]);
            int height=(d[4]+pad < img.rows)?(d[4]-d[2]+pad):(img.rows-d[2]);

            cv::Rect rect(x, y, width, height);
            results.push_back(rect);
        }
    }

    return results;
}

void CarTracker::getTrucks(Mat &img, QString time, QString channelCode, QVector<CarInfo *> &matchList)
{
    bool isPointFrame = false;

    if(truckDetector == nullptr)
    {
        qDebug() << "truck detector init";

        const char* tagName = "TruckDetect";
        string model_file   = util.getValue(tagName, "modelFilePath").toStdString();
        string trained_file = util.getValue(tagName, "trainedFilePath").toStdString();
        string mean_file    = util.getValue(tagName, "meanFilePath").toStdString();
        string mean_value   = util.getValue(tagName, "meanValue").toStdString();
        truckThreshold = util.getValue(tagName, "ConfidenceThreshold").toFloat();

        qDebug() << model_file.c_str() << trained_file.c_str() << mean_value.c_str() << mean_file.c_str() << truckThreshold << "ssssssss";

        truckDetector = new Detector(model_file, trained_file, mean_file, mean_value);
    }

    int pad=10;

    std::vector<vector<float> > detections = truckDetector->Detect(img);

    for (int i = 0; i < detections.size(); ++i) {
        const vector<float>& d = detections[i];
        if (d[0] >= truckThreshold && d[5] == 1) {

            int x=(d[1]-pad > 0)?(d[1]-pad):0;
            int y=(d[2]-pad > 0)?(d[2]-pad):0;
            int width=(d[3]+pad < img.cols)?(d[3]-d[1]+pad):(img.cols-d[1]);
            int height=(d[4]+pad < img.rows)?(d[4]-d[2]+pad):(img.rows-d[2]);

            cv::Rect rect(x, y, width, height);

            if(!checkMatchList(img(rect), matchList) )
            {
                cv::rectangle(img, rect, Scalar(0, 0, 255), 2);
                isPointFrame = true;

                CarInfo *info = new CarInfo;
                info->boundingBox = rect;
                info->matchNums = 0;
                info->feature = getHistFeature(img(rect));
                info->mat = img(rect).clone();

                matchList.push_back(info);

                cv::imshow("not match",img(rect));
                cv::waitKey();
            }
            else
            {
                qDebug() << "------------------------match---------------------";
            }
        }
    }


    if(isPointFrame)
    {
        QString imageUrl = imageSavePath + "/" + QUuid::createUuid().toString().replace("{", "").replace("}", "") + ".jpg";
        cv::imwrite(imageUrl.toStdString(), img);
        QString sql = "insert into carPic_info(channel_id,carPic_time,carPic_url,carPic_type) values('%1',%2,'%3','%4')";

        dbManager.execQuery(sql.arg(channelCode).arg(time).arg(imageUrl).arg("truck"));
    }

}

bool CarTracker::compareCOSLike(float *t1, float *t2, int count)
{
    float dotSum = 0.0f;
    float modeT1 = 0.0f, modeT2 = 0.0f;
    for(int i = 0; i < count; i++)
    {
        dotSum += t1[i]*t2[i];
        modeT1 += t1[i]*t1[i];
        modeT2 += t2[i]*t2[i];
    }
    if(modeT1 == 0 || modeT2 == 0)
        return false;
    else
    {
        double d = acos(dotSum/(sqrt(fabs(modeT1) )*sqrt(fabs(modeT2) ) ) );
        std::cout << d << std::endl;
        return ( d <= 0.2);
    }
}

bool CarTracker::compareShape(std::vector<Prediction> &result,string shape)
{
    for(int i = 0; i < 2; i++)
    {
        if(result.at(i).first == shape)
            return true;

        if(result.at(i).second > 0.5)
            break;
    }

    return false;
}

bool CarTracker::compareColor(std::vector<Prediction> &result, string color)
{
    for(int i = 0; i < 2; i++)
    {
        if(result.at(i).first == color)
            return true;

        if(result.at(i).second > 0.5)
            break;
    }

    return false;
}

float* CarTracker::getHistFeature(Mat image)
{
    if (image.channels() > 1)
        cvtColor(image, image, COLOR_BGR2GRAY);

    cv::Mat histogram;
    const int histSize = 256;
    float range[] = {0, 255};
    const float *ranges[] = {range};
    const int channels = 0;

    cv::calcHist(&image, 1, &channels, cv::Mat(), histogram, 1, &histSize, &ranges[0], true, false);

    float *h = (float*)histogram.data;
    float *hh = new float[256];

    if (h) {
        for (int i = 0; i < 256; ++i) {
            hh[i] = h[i];
        }
    }

    return hh;
}

bool CarTracker::checkMatchList(cv::Mat mat, QVector<CarInfo *> &matchList)
{
    if(matchList.size() == 0)
    {
        std::cout << "empty list" << std::endl;
        return false;
    }

    float *feature = getHistFeature(mat);

    for(int i =0; i < matchList.size(); i++)
    {
        if(compareCOSLike(matchList.at(i)->feature, feature, 256))
        {
            cv::imshow("matched",matchList.at(i)->mat);
            cv::imshow("src",mat);
            cv::waitKey();

            delete matchList.at(i)->feature;
            matchList.at(i)->matchNums = -1;
            matchList.at(i)->feature = feature;

            return true;
        }
    }

    delete feature;

    return false;
}

void CarTracker::excuteMatchList(QVector<CarInfo *> &matchList)
{
    for(QVector<CarInfo *>::Iterator it = matchList.begin(); it != matchList.end(); )
    {
        if((++((*it)->matchNums) ) == 5)
        {
            delete *it;
            it = matchList.erase(it);

            continue;
        }

        it++;
    }
}
