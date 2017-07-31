#include "settingutil.h"
#include <QFile>
#include <QDebug>

SettingUtil::SettingUtil(QString filePath,QObject *parent)
    :QObject(parent)
{
    QFile file(filePath);
    flag=true;
    if(!file.exists())
    {
        flag=false;
        qDebug()<<"error Path "<<filePath;
        return;
    }
    m_setting=new QSettings(filePath,QSettings::IniFormat,this);
}

QString SettingUtil::getValue(QString seg, QString key)
{
    QString tmp = "/"+seg+"/"+key;
    QString value = m_setting->value(tmp,QVariant()).toString();//段名/变量名
    value = value.left(value.indexOf(";"));     //分号之后的是注释，所以键值不能有分号
    return value;
}
