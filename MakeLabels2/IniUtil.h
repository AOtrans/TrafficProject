#ifndef FRONT_UTILITY_INI_UTIL_H
#define FRONT_UTILITY_INI_UTIL_H

#include <QString>
#include <QStringList>
#include <QDebug>
#include <QSettings>

class IniUtil: public QObject
{
    Q_OBJECT
public:
    int visiable;                              //是否显示
    int enabled;                               //是否可用
    int x;                                      //x坐标
	int y;
    int width;                                  //宽度
	int height;
    QString txt;                                //显示的文本
    QString img;                                //显示的图片路径
    QString value;                              //原始读入的值,当ini的值是单一值时,取本属性量
    QSettings *settings;
public:
    IniUtil(QString file);
    bool hasKey(QString key);
    IniUtil(QString file, QString seg, QString key);//ini文件名,段名,键名
    Q_INVOKABLE QString getValue( QString seg, QString key);//ini文件名,段名,键名
    Q_INVOKABLE void setValue( QString seg, QString key, QString  value);//ini文件名,段名,键名)
    Q_INVOKABLE void setValue2( QString seg, QString key, QString  value);//ini文件名,段名,键名--使用文本文件方法更新值
};

#endif
