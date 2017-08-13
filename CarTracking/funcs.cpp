#include "funcs.h"
#include "common.h"
#include <QMutex>

QMutex plateMutex, colorMutex, shapeMutex, videoMutex;

string getPlate(const char *path)
{
    Mat img = imread(path);

    plateMutex.lock();
    CarTracker *c = CarTracker::getInstence();
    string plate = c->getPlate(img);
    plateMutex.unlock();

    return plate;
}

std::vector<Prediction> getColor(const char *path)
{
    Mat img = imread(path);

    colorMutex.lock();
    CarTracker *c = CarTracker::getInstence();
    std::vector<Prediction> result = c->getColor(img,2);
    colorMutex.unlock();

    return result;
}

std::vector<Prediction> getShape(const char *path)
{
    Mat img = imread(path);

    shapeMutex.lock();
    CarTracker *c = CarTracker::getInstence();
    std::vector<Prediction> result = c->getShape(img,2);
    shapeMutex.unlock();

    return result;
}

QString carTrack(QString videoFileName, QString id, QString shape, QString color)
{
    qDebug() << "----------------------------carTrack-----------------------------";
    qDebug() << videoFileName << id << shape << color;

    videoMutex.lock();
    CarTracker *c = CarTracker::getInstence();
    QString fileName = c->carTrack(videoFileName.toStdString(), shape.toStdString(), color.toStdString()).c_str();
    QString sql = "update video_track_info set video_before_url = '%1',video_after_url = '%2' where video_track_id = %3";
    dbManager.execQuery(sql.arg(videoFileName).arg(fileName).arg(id));
    videoMutex.unlock();
    qDebug() << "done";

    return fileName;
}



QString motoTrack(QString videoFileName, QString id, QString shape)
{
    qDebug() << "----------------------------motoTrack-----------------------------";
    qDebug() << videoFileName << id << shape;

//    videoMutex.lock();
//    CarTracker *c = CarTracker::getInstence()
//    QString fileName = c->motoTrack(videoFileName.toStdString(), shape.toStdString()).c_str();
//    QString sql = "update moto_info set before_url = '%1',after_url = '%2' where moto_info_id = %3";
//    dbManager.execQuery(sql.arg(videoFileName).arg(fileName).arg(id));
//    videoMutex.unlock();
return "";
//    qDebug() << "done";
//    return fileName;
}


void truckDetect(QString videoFileName, QString startTime, QString channelCode)
{
    qDebug() << "----------------------------truckTrack-----------------------------";
    qDebug() << videoFileName << startTime << channelCode;

    videoMutex.lock();
    CarTracker *c = CarTracker::getInstence();
    c->truckTrack(videoFileName.toStdString(), startTime.toStdString(), channelCode.toStdString());
    videoMutex.unlock();
    qDebug() << "done";
}

void taxiDetect(QString videoFileName, QString startTime, QString channelCode)
{
    qDebug() << "----------------------------taxiTrack-----------------------------";
    qDebug() << videoFileName << startTime << channelCode;

    videoMutex.lock();
    CarTracker *c = CarTracker::getInstence();
    c->taxiTrack(videoFileName.toStdString(), startTime.toStdString(), channelCode.toStdString());
    videoMutex.unlock();
    qDebug() << "done";
}

void areaCarDetect(QString videoFileName, QString startTime, QString channelCode, QString areas)
{
    qDebug() << "----------------------------areaTrack-----------------------------";
    qDebug() << videoFileName << startTime << channelCode << areas;

    videoMutex.lock();
    CarTracker *c = CarTracker::getInstence();
    c->areaCarTrack(videoFileName.toStdString(), startTime.toStdString(), channelCode.toStdString(), areas.toStdString());
    videoMutex.unlock();
}

void saveShotcut(QString videoFileName, QString id)
{
    qDebug()<<"----------------------------shotcut-----------------------------";
    qDebug()<< videoFileName << id;

    cv::VideoCapture capture(videoFileName.toStdString().c_str());
    QString status = "";
    QString imgPath = "";

    if (!capture.isOpened())
    {
        qDebug() << "capture not opened";
        status = "FALSE";
    }
    else
    {
        Mat frame;

        if(capture.read(frame))
        {
            imgPath = CarTracker::getInstence()->getImageSavePath() + "/" + QUuid::createUuid().toString().replace("{", "").replace("}", "") + ".jpg";
            imwrite(imgPath.toStdString().c_str(), frame);
            status = "TRUE";
        }
        else
        {
            qDebug() << "no frame";
            status = "FALSE";
        }

        capture.release();
    }

    QString sql = "update area_info set video_path = '%1',pic_path = '%2',status = '%3' where id = '%4'";
    dbManager.execQuery(sql.arg(videoFileName).arg(imgPath).arg(status).arg(id));
}

QString trafficStatistics(QString videoFileName, QString id, QString areas)
{
    qDebug() << "----------------------------trafficStatistics-----------------------------";
    qDebug() << videoFileName << id << areas;
    videoMutex.lock();
    CarTracker *c = CarTracker::getInstence();
    QString fileName = c->trafficStatistics(videoFileName.toStdString(), areas.toStdString()).c_str();
//    QString sql = "update video_track_info set video_before_url = '%1',video_after_url = '%2' where video_track_id = %3";
//    dbManager.execQuery(sql.arg(videoFileName).arg(fileName).arg(id));
    videoMutex.unlock();
return "";
//    qDebug() << "done";
//    return fileName;
}
