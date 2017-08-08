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
    videoMutex.lock();
    CarTracker *c = CarTracker::getInstence();
    QString fileName = c->carTrack(videoFileName.toStdString(), shape.toStdString(), color.toStdString(), "", "").c_str();
    QString sql = "update video_track_info set video_before_url = '%1',video_after_url = '%2' where video_track_id = %3";
    dbManager.execQuery(sql.arg(videoFileName).arg(fileName).arg(id));
    videoMutex.unlock();

    return fileName;
}



QString motoTrack(QString videoFileName, QString id, QString shape, QString num, QString helmet)
{
    //todo motoTrack
    videoMutex.lock();
    QString fileName = "";
    QString sql = "update moto_info set before_url = '%1',after_url = '%2' where moto_info_id = %3";
    dbManager.execQuery(sql.arg(videoFileName).arg(fileName).arg(id));
    videoMutex.unlock();

    return fileName;
}


void truckDetect(QString videoFileName, QString startTime, QString channelCode)
{
    videoMutex.lock();
    CarTracker *c = CarTracker::getInstence();
    c->truckTrack(videoFileName.toStdString(), startTime.toStdString(), channelCode.toStdString());
    videoMutex.unlock();
}

void taxiDetect(QString videoFileName, QString startTime, QString channelCode)
{
    videoMutex.lock();
    CarTracker *c = CarTracker::getInstence();
    c->taxiTrack(videoFileName.toStdString(), startTime.toStdString(), channelCode.toStdString());
    videoMutex.unlock();
}

void areaCarDetect(QString videoFileName, QString startTime, QString channelCode, QString areas)
{
    videoMutex.lock();
    CarTracker *c = CarTracker::getInstence();
    c->areaCarTrack(videoFileName.toStdString(), startTime.toStdString(), channelCode.toStdString(), areas.toStdString());
    videoMutex.unlock();
}
