#ifndef FUNCS_H
#define FUNCS_H
#include "CarTracker/cartracker.h"

string getPlate(const char* path);

std::vector<Prediction> getColor(const char *path);

std::vector<Prediction> getShape(const char *path);

QString carTrack(QString videoFileName, QString id, QString shape, QString color);

QString motoTrack(QString videoFileName, QString id, QString shape);

void truckDetect(QString videoFileName, QString startTime, QString channelCode);

void taxiDetect(QString videoFileName, QString startTime, QString channelCode);

void areaCarDetect(QString videoFileName, QString startTime, QString channelCode, QString areas);

void saveShotcut(QString videoFileName, QString id);

QString trafficStatistics(QString videoFileName, QString id, QString areas);
#endif // FUNCS_H
