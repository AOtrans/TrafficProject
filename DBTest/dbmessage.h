#ifndef DBMESSAGE_H
#define DBMESSAGE_H

#include<QString>
#include"common.h"

class DBMessage
{
public:
    DBMessage();
    DBMessage(QString dbmsName,QString hostName,QString port,QString dbName,QString account,QString password,int tryReConnectTimes=5,int tryReQueryTimes=3);
    QString getDbmsName() const;
    void setDbmsName(const QString &value);

    QString getHostName() const;
    void setHostName(const QString &value);

    QString getPort() const;
    void setPort(const QString &value);

    QString getDbName() const;
    void setDbName(const QString &value);

    QString getAccount() const;
    void setAccount(const QString &value);

    QString getPassword() const;
    void setPassword(const QString &value);

    int getTryReConnectTimes() const;
    void setTryReConnectTimes(int tryReConnectTimes);

    int getTryReQueryTimes() const;
    void setTryReQueryTimes(int tryReQueryTimes);

private:
    QString m_dbmsName;
    QString m_hostName;
    QString m_port;
    QString m_dbName;
    QString m_account;
    QString m_password;
    int m_tryReConnectTimes;
    int m_tryReQueryTimes;
};

#endif // DBMESSAGE_H
