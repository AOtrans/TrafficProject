#ifndef DBCOMMON_H
#define DBCOMMON_H
#include "unistd.h"
#include<QDebug>
#include<QDomDocument>
#define DEBUG(s) qDebug()<<"LINE:"<<__LINE__<<"--FUNCTION:"<<__FUNCTION__<<"--Time:"<<QTime::currentTime().toString("－hh:mm:ss.zzz")<<"("<<s<<")";
namespace dbConfig
{
    const QString SegName="DBConfig";
    const QString DBmsName="dbmsName";
    const QString HostName="hostName";
    const QString Port="port";
    const QString DBName="dbName";
    const QString Account="account";
    const QString Password="password";
    const QString TryReConnectTimes="tryReConnectTimes";
    const QString TryReQueryTimes="tryReQueryTimes";
    const QString MYSQL="MYSQL";
}
#endif // DBCOMMON_H
