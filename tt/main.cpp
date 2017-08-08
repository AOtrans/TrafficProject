#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QDebug>
#include <QNetworkReply>
#include<QNetworkRequest>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    for(int i=0;i<5;i++)
    {
        QNetworkRequest request;
        request.setUrl(QUrl("http://127.0.0.1:58890/Plate/_home_zg_traffic_temp_plate.jpg"));
        //创建一个管理器
        static QNetworkAccessManager *manager = new QNetworkAccessManager();
        //发送GET请求
        QNetworkReply *reply = manager->get(request);
    }
    return a.exec();
}
