#include "settingutil.h"
SettingUtil util("/home/zg/traffic/QtProject/CarTracking/config.ini");
#include "http/httpserver.h"
#include <QTextCodec>
#include <QtCore/QCoreApplication>
#include <QMutex>
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
    HttpServer server;
    if(!server.listen(58890))
    {
        qDebug()<<"启动服务器失败！";
        exit(-1);
    }
    qDebug()<<"启动服务器成功！";
    return a.exec();
}
