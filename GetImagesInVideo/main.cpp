#include "widget.h"
#include <QApplication>
#include <QDebug>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    Widget w;
//    w.show();
    cv::VideoCapture capture(videoFile.toStdString());
    if (!capture.isOpened())
    {
        ui->tbMessage->append("fail to open");
        fail<<videoFile<<"\n";
        continue;
    }
    else
    {
        success<<videoFile<<"\n";
    }

    cv::Mat frame;

    while (capture.read(frame))
    {

    }
    return a.exec();
}
