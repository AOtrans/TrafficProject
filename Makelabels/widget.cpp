#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    shapeGroup =new QButtonGroup(this);
    colorGroup =new QButtonGroup(this);
    iniUtil=new IniUtil("./config.ini");

    shapeGroup->addButton(ui->radioButton_0,0);
    shapeGroup->addButton(ui->radioButton_1,1);
    shapeGroup->addButton(ui->radioButton_2,2);
    shapeGroup->addButton(ui->radioButton_3,3);
    shapeGroup->addButton(ui->radioButton_4,4);
    shapeGroup->addButton(ui->radioButton_5,5);
    shapeGroup->addButton(ui->radioButton_6,6);
    shapeGroup->addButton(ui->radioButton_7,7);
    shapeGroup->addButton(ui->radioButton_8,8);
    shapeGroup->addButton(ui->radioButton_9,9);
    shapeGroup->addButton(ui->radioButton,10);


    colorGroup->addButton(ui->radioButton_10,0);
    colorGroup->addButton(ui->radioButton_11,1);
    colorGroup->addButton(ui->radioButton_12,2);
    colorGroup->addButton(ui->radioButton_13,3);
    colorGroup->addButton(ui->radioButton_14,4);
    colorGroup->addButton(ui->radioButton_15,5);
    colorGroup->addButton(ui->radioButton_16,6);
    colorGroup->addButton(ui->radioButton_17,7);
    colorGroup->addButton(ui->radioButton_18,8);
    colorGroup->addButton(ui->radioButton_19,9);

    init();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::init()
{
    currentIndex=0;
    ui->pbPre->setEnabled(false);
    ui->pbNext->setEnabled(false);
    ui->radioButton_0->setChecked(true);
    ui->radioButton_10->setChecked(true);
}

void Widget::changeImg(QString filePath)
{
    if(currentFile!="")
    {
        saveLabel();
    }
    QImage img(filePath);
    ui->imageLabel->setPixmap(QPixmap::fromImage(img));
    currentFile=filePath;
    ui->leFileName->setText(currentFile);
    checkLabel();
}

void Widget::closeEvent(QCloseEvent *event)
{
    if(dirPath!="")
    {
        iniUtil->setValue(dirPath.replace(QRegExp("\\"),"#").replace(QRegExp("/"),"#"),"currentIndex",QString::number(currentIndex));
        saveLabel();
    }
    QWidget::closeEvent(event);
}

void Widget::saveLabel()
{
    QFile file(currentFile.left(currentFile.indexOf("."))+".txt");
    file.open(QIODevice::ReadWrite);
    QTextStream ts(&file);
    int label1=shapeGroup->checkedId();
    int label2=colorGroup->checkedId();
    ts<<label1<<"\n"<<label2;
    file.close();
}

void Widget::checkLabel()
{
    QFile file(currentFile.left(currentFile.indexOf("."))+".txt");
    file.open(QIODevice::ReadWrite);
    QTextStream ts(&file);
    for(int i=0;i<2;i++)
    {
        if(i==0)
            shapeGroup->button(ts.readLine().toInt())->setChecked(true);
        else
            colorGroup->button(ts.readLine().toInt())->setChecked(true);
    }
}

void Widget::on_pbChooseDir_clicked()
{
    dirPath=QFileDialog::getExistingDirectory(this,"Choose images Dir path","./");
    if(dirPath!="")
    {
        ui->leDirName->setText(dirPath);
        QDir dir(dirPath);
        QStringList filter;
        filter<<"*.jpg"<<"*.png"<<"*.jpeg"<<"*.bmp";
        ls=dir.entryInfoList(filter,QDir::Files,QDir::Name);
        if(ls.size()==0)
        {
            dirPath="";
            return;
        }
        ui->pbPre->setEnabled(true);
        ui->pbNext->setEnabled(true);
        QString temp;
        if(""!=(temp=iniUtil->getValue(dirPath.replace(QRegExp("\\"),"#").replace(QRegExp("/"),"#"),"currentIndex")))
        {
            currentIndex=qMin(temp.toInt(),(ls.size()-1));
            ui->label_now->setText("now:"+QString::number(currentIndex+1));
        }
        else
        {
            currentIndex=0;
            ui->label_now->setText("now:1");
        }
        ui->label_total->setText("total:"+QString::number(ls.size()));
        changeImg(ls.at(currentIndex).filePath());
        if(currentIndex==0)
            ui->pbPre->setEnabled(false);
        if(currentIndex==(ls.size()-1))
            ui->pbNext->setEnabled(false);
    }
}

void Widget::on_pbPre_clicked()
{
    if(currentIndex==0)
        return;
    else
        --currentIndex;
    ui->label_now->setText("now:"+QString::number(currentIndex+1));
    if(currentIndex==0)
    {
        ui->pbPre->setEnabled(false);
    }
    ui->pbNext->setEnabled(true);
    changeImg(ls.at(currentIndex).filePath());
}

void Widget::on_pbNext_clicked()
{
    if(currentIndex==(ls.size()-1))
        return;
    else
        ++currentIndex;
    if(currentIndex==(ls.size()-1))
    {
        ui->pbNext->setEnabled(false);
    }
    ui->label_now->setText("now:"+QString::number(currentIndex+1));
    ui->pbPre->setEnabled(true);
    changeImg(ls.at(currentIndex).filePath());
}
