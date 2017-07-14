#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    successFile.setFileName("./success.txt");
    failFile.setFileName("./fail.txt");
    currentDir=".";
}

Widget::~Widget()
{
    delete ui;
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
                    cv::Mat carImg=frame(*it).clone();
                    QString tag=getShape(carImg);
                    imageCount++;
                    groupCount=imageCount/3000;
                    QString dir=currentDir+"/"+QString::number(groupCount)+"/"+tag;
                    if(!QDir(dir).exists())
                        QDir(currentDir).mkpath(dir);
                    cv::imwrite((dir+"/"+QString::number(imageCount)+".jpg").toStdString(),frame(*it));
                }
            }
            writer.release();
            capture.release();
        }
        successFile.close();
        failFile.close();
    }
}
