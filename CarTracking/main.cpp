#include "settingutil.h"
#include "DB/dbmanager.h"
#include "http/httpserver.h"
#include <QTextCodec>
#include <QtCore/QCoreApplication>
#include <QMutex>


SettingUtil util("/home/zg/traffic/QtProject/CarTracking/config.ini");
DBManager dbManager(0,&util);
QMutex mutex;

void initCodec()
{
    QTextCodec *codec = QTextCodec::codecForName("System");
    QTextCodec::setCodecForLocale(codec);
    //QTextCodec::setCodecForCStrings(codec);
   // QTextCodec::setCodecForTr(codec);
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    initCodec();

    if(!dbManager.connect())
    {
        qDebug()<<"DB Connect false";
        exit(-1);
    }
    qDebug()<<"DB connect success";

    HttpServer server;
    if(!server.listen(58890))
    {
        qDebug()<<"open service false！";
        exit(-1);
    }
    qDebug()<<"open service success！";

    return a.exec();
}
