#ifndef COMMON_H
#define COMMON_H
#include <unistd.h>
#include<QDebug>
#include<QDomDocument>
#define DEBUG(s) qDebug()<<"LINE:"<<__LINE__<<"--FUNCTION:"<<__FUNCTION__<<"--Time:"<<QTime::currentTime().toString("－hh:mm:ss.zzz")<<"("<<s<<")";
namespace dbConfig
{
    static const QString SegName="DBConfig";
    static const QString DBmsName="dbmsName";
    static const QString HostName="hostName";
    static const QString Port="port";
    static const QString DBName="dbName";
    static const QString Account="account";
    static const QString Password="password";
    static const QString TryReConnectTimes="tryReConnectTimes";
    static const QString TryReQueryTimes="tryReQueryTimes";
    static const QString MYSQL="MYSQL";
}
#endif // COMMON_H
