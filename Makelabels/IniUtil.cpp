#include "IniUtil.h"
#include "QString"
#include <QSettings>
#include <QDebug>
#include <QObject>
#include <QTextCodec>
#include <QFile>
#include <QTextStream>

IniUtil::IniUtil(QString file)
{
   settings = new QSettings(file, QSettings::IniFormat, 0);
//   QTextCodec *codec = QTextCodec::codecForName("system");///0807修改codecForName-改为system 原来是GB2312
//   QTextCodec::setCodecForCStrings(codec);
//   QTextCodec::setCodecForLocale(codec);
//   QTextCodec::setCodecForTr(codec);
   //   settings->setIniCodec(codec);  //支持中文
}

bool IniUtil::hasKey(QString key)
{
    return settings->contains(key);
}

//读ini文件中指定段set中指定键key的值
IniUtil::IniUtil(QString file, QString seg, QString key)
{
    QString tmp = "/"+seg+"/"+key;
    value = settings->value(tmp,NULL).toString();//段名/变量名
    value = value.left(value.indexOf(";"));     //分号之后的是注释，所以键值不能有分号
    //qDebug()<<"value="<<value;
    switch(value.count(","))
    {
    case 0:  //只有一个值
        break;
    case 7: //窗口控件，依次是：可见，可用，x,y,w,h,txt,img
        QStringList slValue = value.split(",");
        visiable=slValue[0].toUInt();
        enabled = slValue[1].toUInt();
        x = slValue[2].toUInt();
        y = slValue[3].toUInt();
        width = slValue[4].toUInt();
        height = slValue[5].toUInt();
        txt = slValue[6];
        img = slValue[7].toLatin1().toStdString().c_str();
        break;

    }
    //qDebug()<<"visiable="<<visiable<<",x="<<x<<",y="<<y<<",txt="<<txt<<",img="<<img;
}

//读ini文件中指定段set中指定键key的值
QString IniUtil::getValue( QString seg, QString key)
{
    QString tmp = "/"+seg+"/"+key;
    value = settings->value(tmp,"").toString();//段名/变量名
    value = value.left(value.indexOf(";"));     //分号之后的是注释，所以键值不能有分号
    //qDebug()<<"value="<<value;
    switch(value.count(","))
    {
    case 0:  //只有一个值
        break;
    case 7: //窗口控件，依次是：可见，可用，x,y,w,h,txt,img
        QStringList slValue = value.split(",");
        visiable=slValue[0].toUInt();
        enabled = slValue[1].toUInt();
        x = slValue[2].toUInt();
        y = slValue[3].toUInt();
        width = slValue[4].toUInt();
        height = slValue[5].toUInt();
        txt = slValue[6];
        img = slValue[7].toLatin1().toStdString().c_str();
        break;

    }
    return value;
    qDebug()<<"visiable="<<visiable<<",x="<<x<<",y="<<y<<",txt="<<txt<<",img="<<img;
}


//ini文件名,段名,键名,新值
void IniUtil::setValue(QString seg, QString key, QString  value)
{
    QString tmp = "/"+seg+"/"+key;
    qDebug()<<__FUNCTION__<<"-tmp="<<tmp<<"-value="<<value;
    settings->setValue(tmp, QString::fromLocal8Bit(value.toLocal8Bit().data() ) );
}

void IniUtil::setValue2(QString seg, QString key, QString value)
{
    bool segFound =false;
    bool nextSegFount = false;
    QFile file(settings->fileName());
    QFile fileNew ( settings->fileName()+".txt");
    if (file.open(QFile::ReadOnly)  &&fileNew.open(QFile::WriteOnly))
    {
        QTextStream floStream(&file);
        QTextStream newStream(&fileNew);
        QString line;
        QTextCodec *codec=QTextCodec::codecForName("system");
        floStream.setCodec(codec);
        newStream.setCodec(codec);
        while ( floStream.atEnd()==0 )
        {
            line = codec->fromUnicode(floStream.readLine());
            //qDebug()<<__FUNCTION__<<"-"<<__LINE__<<"-line="<<line;
            if ( line.indexOf("[") >=0 && line.indexOf(seg)>0 ) {
                segFound = true;
                qDebug()<<__FUNCTION__<<"-"<<__LINE__<<"-找到了段";
                newStream<<line<<"\r\n";  //写原来的 [seg]
                newStream<<key<<"=\""<<value<<"\"\r\n"; //写添加的值
                continue;
            }

            if ( line.indexOf("[") >=0 ) {
                qDebug()<<__FUNCTION__<<"-"<<__LINE__<<"-到了新段";
                nextSegFount = true;  //新段内，相同的键值，由于段不同，保持原样
            }

            if (!nextSegFount && line.trimmed().startsWith(key) )
            { //在没有到下一个段之前,找到了key,表示是以前有的,需要删除它
                qDebug()<<__FUNCTION__<<"-"<<__LINE__<<"-找到原值";

                continue;
            }
            newStream<<line<<"\r\n";

        }
        file.close();
        fileNew.close();

        QFile fileOri(settings->fileName());
        if (fileOri.open(QFile::WriteOnly)  &&fileNew.open(QFile::ReadOnly)) {
            fileOri.write(fileNew.readAll());
        }
        fileOri.close();
        fileNew.close();
    }
}

