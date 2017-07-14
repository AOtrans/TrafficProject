#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::doNormal(QDir &dir)
{
    QFileInfoList ls=dir.entryInfoList();
    foreach(QFileInfo info,ls)
    {
        if(info.filePath().contains("\\..")||info.filePath().contains("\\.")||info.filePath().contains("/..")||info.filePath().contains("/."))
        {

        }
        else
        {
            if(info.isDir())
            {
                qDebug()<<info.filePath();
                QDir subDir(info.filePath());
                doNormal(subDir);
            }
            else
            {
                qDebug()<<info.filePath();
                if(info.filePath().contains(".jpg")||info.filePath().contains(".jpeg")||info.filePath().contains(".png"))
                {
                    cv::Mat src=cv::imread(info.filePath().toStdString().c_str());
                    cv::resize(src,src,cv::Size(256,256));
                    cv::imwrite(info.filePath().toStdString().c_str(),src);
                }
            }
        }
    }
}

void MainWindow::dosubDir(QString prefix,int label, QMap<QString, int> &typeMap,QMap<QString,int> &nameMap, QTextStream &ts, QDir &subDir ,bool ifDo ,bool ifDoSub)
{
    QFileInfoList ls=subDir.entryInfoList();
    foreach(QFileInfo info,ls)
    {
        if(info.filePath().contains("\\..")||info.filePath().contains("\\.")||info.filePath().contains("/..")||info.filePath().contains("/."))
        {

        }
        else
        {
            if(info.isDir())
            {
                if(typeMap.find(info.fileName())==typeMap.end())
                {
                    typeMap.insert(info.fileName(),typeMap.size());
                }
                QDir subsubDir(info.filePath());
                if(ifDo)
                {
                    if(ifDoSub)
                    {
                        dosubsubDir(prefix+"_"+info.fileName(),label,typeMap.value(info.fileName()),nameMap,ts,subsubDir);
                    }
                    else
                    {
                        dosubsubDir(prefix+"_"+info.fileName(),label,typeMap.value(info.fileName()),nameMap,ts,subsubDir,false);
                    }
                }
                else
                {
                    if(ifDoSub)
                    {
                        dosubsubDir(prefix+"_"+info.fileName(),label,-1,nameMap,ts,subsubDir);
                    }
                    else
                    {
                        dosubsubDir(prefix+"_"+info.fileName(),label,-1,nameMap,ts,subsubDir,false);
                    }
                }
            }
        }
    }
}

void MainWindow::dosubsubDir(QString prefix,int label, int label2, QMap<QString, int> &nameMap, QTextStream &ts, QDir &subsubDir, bool ifDo)
{
    QFileInfoList ls=subsubDir.entryInfoList();
    foreach(QFileInfo info,ls)
    {
        if(info.filePath().contains("\\..")||info.filePath().contains("\\.")||info.filePath().contains("/..")||info.filePath().contains("/."))
        {

        }
        else
        {
            if(info.isDir())
            {
                if(nameMap.find(prefix+"_"+info.fileName())==nameMap.end())
                {
                    nameMap.insert(prefix+"_"+info.fileName(),nameMap.size());
                }
                QDir subsubsubDir(info.filePath()+"/shape");
                if(ifDo)
                {
                    doMultiTxt(prefix+"_"+info.fileName(),label,label2,nameMap.value(prefix+"_"+info.fileName()),ts,subsubsubDir);
                }
                else
                {
                    doMultiTxt(prefix+"_"+info.fileName(),label,label2,-1,ts,subsubsubDir);
                }
            }
        }
    }
}

void MainWindow::on_pushButton_clicked()
{
    QFileDialog *dialog=new QFileDialog(this);
    dialog->setFileMode(QFileDialog::ExistingFile);
    fileName=dialog->getOpenFileName();
    ui->lineEdit->setText(fileName);
}

void MainWindow::on_pushButton_2_clicked()
{
    QFileDialog *dialog=new QFileDialog(this);
    dialog->setFileMode(QFileDialog::DirectoryOnly);
    dirName=dialog->getExistingDirectory();
    ui->lineEdit_2->setText(dirName);
}

void MainWindow::on_pushButton_3_clicked()
{
    if(ui->lineEdit->text()!=""&&ui->lineEdit_2->text()!="")
    {
        QFile file(fileName);
        file.open(QIODevice::ReadWrite|QIODevice::Append);
        QTextStream ts(&file);

        QDir dir(dirName);
        QStringList filter;
        filter<<"*.jpg"<<"*.png"<<"*.jpeg";
        QFileInfoList ls=dir.entryInfoList(filter);
        foreach(QFileInfo info,ls)
        {
            ts<<info.filePath()<<" "<<ui->spinBox->text().trimmed()<<"\n";
        }
        file.close();
        qDebug()<<"ok";
    }
}

void MainWindow::on_normalized_clicked()
{
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    QString dirName=dialog.getExistingDirectory();
    QDir dir(dirName);
    QStringList filter;
    filter<<"*.jpg"<<"*.png"<<"*.jpeg";
    QFileInfoList ls=dir.entryInfoList(filter);
    int i=1;
    foreach(QFileInfo info,ls)
    {
        QFile file(info.filePath());
        qDebug()<<info.filePath();
        if(file.exists())
            file.rename(dirName+"/"+QString::number(i++)+"a.jpeg");
    }

    QDir dir2(dirName);
    QFileInfoList ls2=dir2.entryInfoList(filter);
    i=1;
    foreach(QFileInfo info,ls2)
    {
        QFile file(info.filePath());
        qDebug()<<info.filePath();
        cv::Mat src=cv::imread(info.filePath().toStdString().c_str());
        cv::resize(src,src,cv::Size(256,256));
        cv::imwrite(info.filePath().toStdString().c_str(),src);
        if(file.exists())
            file.rename(dirName+"/"+QString::number(i++)+".jpeg");
    }
}

void MainWindow::on_pushButton_4_clicked()
{
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    QString dirName=dialog.getExistingDirectory();
    QDir dir(dirName);
    doNormal(dir);
}


void MainWindow::on_pushButton_5_clicked()
{
    bool doit=false;
    bool dosub=false;
    bool dosubsub=true;
    QMap<QString,int> shapeMap;
    QMap<QString,int> typeMap;
    QMap<QString,int> nameMap;
    QFile file("all/multiLabel.txt");
    file.open(QIODevice::ReadWrite);
    QTextStream ts(&file);
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    QString dirName=dialog.getExistingDirectory();
    QDir dir(dirName);

    QFileInfoList ls=dir.entryInfoList();
    foreach(QFileInfo info,ls)
    {
        if(info.filePath().contains("\\..")||info.filePath().contains("\\.")||info.filePath().contains("/..")||info.filePath().contains("/."))
        {

        }
        else
        {
            if(info.isDir())
            {
                shapeMap.insert(info.fileName(),shapeMap.size());
                QDir subDir(info.filePath());
                if(doit)
                {
                    dosubDir(info.fileName(),shapeMap.size()-1,typeMap,nameMap,ts,subDir,dosub,dosubsub);
                }
                else
                {
                    dosubDir(info.fileName(),-1,typeMap,nameMap,ts,subDir,dosub,dosubsub);
                }
            }
        }
    }
    file.close();


    QFile labelFile1("label1.txt");
    QFile labelFile2("label2.txt");
    QFile labelFile3("label3.txt");
    labelFile1.open(QIODevice::ReadWrite);
    labelFile2.open(QIODevice::ReadWrite);
    labelFile3.open(QIODevice::ReadWrite);
    QTextStream ts1(&labelFile1);
    QTextStream ts2(&labelFile2);
    QTextStream ts3(&labelFile3);


    for(int i=0;i<shapeMap.size();i++)
    {
        ts1<<i<<" "<<shapeMap.key(i)<<"\n";
    }
    for(int i=0;i<typeMap.size();i++)
    {
        ts2<<i<<" "<<typeMap.key(i)<<"\n";
    }
    for(int i=0;i<nameMap.size();i++)
    {
        ts3<<i<<" "<<nameMap.key(i)<<"\n";
    }
    labelFile1.close();
    labelFile2.close();
    labelFile3.close();

}

void MainWindow::doMultiTxt(QString prefix,int label1,int label2,int label3,QTextStream& ts,QDir &subsubsubDir)
{
    qDebug()<<prefix<<" "<<label1<<" "<<label2<<" "<<label3;
    QStringList filter;
    filter<<"*.jpg"<<"*.png"<<"*.jpeg";
    QFileInfoList ls=subsubsubDir.entryInfoList(filter);
    foreach(QFileInfo info,ls)
    {
        ts<<info.filePath()<<((label1==-1)?QString(""):(" "+QString::number(label1)))<<((label2==-1)?"":(" "+QString::number(label2)))<<((label3==-1)?"":(" "+QString::number(label3)))<<"\n";
    }
}

void MainWindow::on_pbFileTxt_clicked()
{
    QFile shapefile1("./trainShape.txt");
    QFile shapefile2("./valShape.txt");
    shapefile1.open(QIODevice::ReadWrite);
    shapefile2.open(QIODevice::ReadWrite);
    QTextStream shapets1(&shapefile1);
    QTextStream shapets2(&shapefile2);

    QFile colorfile1("./trainColor.txt");
    QFile colorfile2("./valColor.txt");
    colorfile1.open(QIODevice::ReadWrite);
    colorfile2.open(QIODevice::ReadWrite);
    QTextStream colorts1(&colorfile1);
    QTextStream colorts2(&colorfile2);
    int *a=new int[11];
    for(int i=0;i<11;i++)
        a[i]=0;
    int *b=new int[10];
    for(int i=0;i<10;i++)
        b[i]=0;
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    QString dirName=dialog.getExistingDirectory();
    QDir dir(dirName);
    QFileInfoList ls=dir.entryInfoList(QStringList(),QDir::Dirs|QDir::NoDotAndDotDot);
    foreach(QFileInfo info,ls)
    {
        QDir dir2(info.filePath());
        QStringList filter;
        filter<<"*.jpg"<<"*.png"<<"*.jpeg";
        QFileInfoList ls2=dir2.entryInfoList(filter,QDir::Files);
        {
            foreach(QFileInfo info2,ls2)
            {
                QFile temp(info2.filePath().left(info2.filePath().indexOf("."))+".txt");
                if(temp.exists())
                {
                    temp.open(QIODevice::ReadWrite);
                    QTextStream tsTemp(&temp);
                    for(int i=0;i<2;i++)
                    {
                        QString str=tsTemp.readLine();
                        if(i==0)
                        {
                            shapets1<<info2.filePath()<<" "<<str<<"\n";
                            int index=str.toInt();
                            a[index]++;
                        }
                        else
                        {
                            colorts1<<info2.filePath()<<" "<<str<<"\n";
                            int index=str.toInt();
                            b[index]++;
                        }
                    }
                }
            }

        }
    }
    shapefile1.close();
    shapefile2.close();
    colorfile1.close();
    colorfile2.close();
    for(int i=0;i<11;i++)
        qDebug()<<a[i];
    qDebug()<<"-------------------";
    for(int i=0;i<10;i++)
        qDebug()<<b[i];
    qDebug()<<"ok";
}
