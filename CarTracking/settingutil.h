#ifndef SETTINGUTIL_H
#define SETTINGUTIL_H

#include <QObject>
#include <QSettings>
#include <QDir>

class SettingUtil : public QObject
{
    Q_OBJECT
public:
    explicit SettingUtil(QString filePath,QObject *parent = 0);
    QString getValue(QString seg,QString key);
private:
    QSettings *m_setting;
    bool flag;
signals:

public slots:
};

#endif // SETTINGUTIL_H
