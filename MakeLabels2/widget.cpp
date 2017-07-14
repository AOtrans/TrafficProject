#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    group1 =new QButtonGroup(this);
    group2 =new QButtonGroup(this);
    group3=new QButtonGroup(this);
    iniUtil=new IniUtil("./config.ini");
    ui->pbNext->setShortcut(Qt::Key_D);
    ui->pbPre->setShortcut(Qt::Key_A);

    group1->addButton(ui->radioButton_1,0);
    group1->addButton(ui->radioButton_2,1);
    group1->addButton(ui->radioButton_3,2);

    group2->addButton(ui->radioButton_4,0);
    group2->addButton(ui->radioButton_5,1);
    group2->addButton(ui->radioButton_6,2);

    group3->addButton(ui->radioButton_7,0);
    group3->addButton(ui->radioButton_8,1);
    group3->addButton(ui->radioButton_9,2);

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
    ui->pbDelete->setEnabled(false);
    ui->radioButton_1->setChecked(true);
    ui->radioButton_4->setChecked(true);
    ui->radioButton_7->setChecked(true);
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
    int label1=group1->checkedId();
    int label2=group2->checkedId();
    int label3=group3->checkedId();
    ts<<label1<<"\n"<<label2<<"\n"<<label3;
    file.close();
}

void Widget::checkLabel()
{
    QFile file(currentFile.left(currentFile.indexOf("."))+".txt");
    file.open(QIODevice::ReadWrite);
    QTextStream ts(&file);
    for(int i=0;i<3;i++)
    {
        if(i==0)
            group1->button(ts.readLine().toInt())->setChecked(true);
        else if(i==1)
            group2->button(ts.readLine().toInt())->setChecked(true);
        else
            group3->button(ts.readLine().toInt())->setChecked(true);
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
        ui->pbDelete->setEnabled(true);
        QString temp;
        if(""!=(temp=iniUtil->getValue(dirPath.replace(QRegExp("\\"),"#").replace(QRegExp("/"),"#"),"currentIndex")))
        {
            currentIndex=qMin(temp.toInt(),(ls.size()-1));
            currentIndex=qMax(currentIndex,0);
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

void Widget::on_pbDelete_clicked()
{
    if(currentIndex>=ls.size())
        return;
    QFile file(currentFile);
    file.remove();

    QFile file2(currentFile.left(currentFile.indexOf("."))+".txt");
    if(file2.exists())
        file2.remove();
    currentFile="";
    ls.removeAt(currentIndex);
    if(currentIndex==ls.size())
    {
        currentIndex--;
    }
    if(currentIndex==(ls.size()-1))
    {
        ui->pbNext->setEnabled(false);
    }
    ui->label_total->setText("total:"+QString::number(ls.size()));
    ui->label_now->setText("now:"+QString::number(currentIndex+1));
    if(ls.size()<=0)
    {
        ui->imageLabel->clear();
        init();
        return;
    }
    changeImg(ls.at(currentIndex).filePath());

}
