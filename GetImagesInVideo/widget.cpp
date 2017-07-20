#include "widget.h"
#include "ui_widget.h"
#include <QDebug>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    successFile.setFileName("./success.txt");
    failFile.setFileName("./fail.txt");
    currentDir="/home/zg/1T/samples";
    carDetector = nullptr;
}

Widget::~Widget()
{
    delete ui;
}

vector<Rect> Widget::getCars(Mat &img)
{
    vector<Rect> results;
    if(carDetector==nullptr)
    {
        confidenceThreshold = 0.25;
        carDetector=new Detector("/home/zg/1T/car_detection/files/model/SSD_300x300/deploy.prototxt", "/home/zg/1T/car_detection/files/snapshot/SSD_300x300/VGG_VOC0712_SSD_300x300_iter_40587.caffemodel", "", "104,117,123");
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

bool Widget::compareRect(Rect r, Rect rOther)
{
    int x0 = std::max(r.x , rOther.x);
    int x1 = std::min(r.x + r.width, rOther.x + rOther.width);
    int y0 = std::max(r.y, rOther.y);
    int y1 = std::min(r.y + r.height, rOther.y + rOther.height);

    if (x0 >= x1 || y0 >= y1) return false;

    float areaInt = (x1-x0)*(y1-y0);
    return (areaInt/((float)r.width*r.height+(float)rOther.width*rOther.height-areaInt)>0.6);
}

void Widget::on_pbChooseFile_clicked()
{
    videoListFileName=QFileDialog::getOpenFileName(this,"choose input file",".",tr("Images (*.txt)"));
    if(videoListFileName!="")
    {
        ui->leFileName->setText(videoListFileName);
    }
}

void Widget::on_pbBegin_clicked()
{
    if(videoListFileName=="")
    {
        QMessageBox::warning(this,"提示","未输入视频列表文件名");
    }
    else
    {
        QFile file(videoListFileName);
        file.open(QIODevice::ReadOnly);
        QTextStream ts(&file);

        QString videoFile;

        successFile.open(QIODevice::ReadWrite|QIODevice::Append);
        failFile.open(QIODevice::ReadWrite|QIODevice::Append);
        QTextStream success(&successFile);
        QTextStream fail(&failFile);

        int imageCount=0;
        int groupCount=-1;
        while ((videoFile=ts.readLine())!="")
        {
            ui->tbMessage->append(QString()+"-----------------"+videoFile+"------------------");
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

            ui->tbMessage->append(QString("frameconut: ")+QString::number(capture.get(CV_CAP_PROP_FRAME_COUNT)));
            cv::Mat frame;

            int cross=ui->sbCross->value()+1;
            int count=0;
            while (capture.read(frame))
            {
                if((count++)%cross!=0)
                    continue;

                vector<cv::Rect> &&cars=getCars(frame);
                for(vector<cv::Rect>::iterator it=cars.begin();it!=cars.end();it++)
                {
                    if(matchList.size()!=0)
                    {
                        bool jump=false;
                        for(int i=0;i<matchList.size();i++)
                        {
                            if(compareRect(*it,matchList.at(i)))
                            {
//                                qDebug()<<"jump";
//                                imshow("dd",frame(*it));
//                                cv::waitKey();
                                jump=true;
                                break;
                            }
                        }
                        if(jump)
                            continue;
                    }
                    imageCount++;
                    groupCount=imageCount/3000;
                    QString dir=currentDir+"/"+QString::number(groupCount);
                    if(!QDir(dir).exists())
                        QDir(currentDir).mkpath(dir);
                    cv::imwrite((dir+"/"+QString::number(imageCount)+".jpg").toStdString(),frame(*it));
                }
                matchList = cars;
                qApp->processEvents();
            }
            capture.release();
        }
        successFile.close();
        failFile.close();
    }
    qDebug()<<"ok";
}
