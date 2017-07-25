#include "dbmessage.h"

DBMessage::DBMessage()
{

}

DBMessage::DBMessage(QString dbmsName, QString hostName, QString port, QString dbName, QString account, QString password,int tryReConnectTimes,int tryReQueryTimes)
    :m_dbmsName(dbmsName),
      m_hostName(hostName),
      m_port(port),
      m_dbName(dbName),
      m_account(account),
      m_password(password),
      m_tryReConnectTimes(tryReConnectTimes),
      m_tryReQueryTimes(tryReQueryTimes)
{

}

QString DBMessage::getDbmsName() const
{
    return m_dbmsName;
}

void DBMessage::setDbmsName(const QString &value)
{
    m_dbmsName = value;
}

QString DBMessage::getHostName() const
{
    return m_hostName;
}

void DBMessage::setHostName(const QString &value)
{
    m_hostName = value;
}

QString DBMessage::getPort() const
{
    return m_port;
}

void DBMessage::setPort(const QString &value)
{
    m_port = value;
}

QString DBMessage::getDbName() const
{
    return m_dbName;
}

void DBMessage::setDbName(const QString &value)
{
    m_dbName = value;
}

QString DBMessage::getAccount() const
{
    return m_account;
}

void DBMessage::setAccount(const QString &value)
{
    m_account = value;
}

QString DBMessage::getPassword() const
{
    return m_password;
}

void DBMessage::setPassword(const QString &value)
{
    m_password = value;
}

int DBMessage::getTryReConnectTimes() const
{
    return m_tryReConnectTimes;
}

void DBMessage::setTryReConnectTimes(int tryReConnectTimes)
{
    m_tryReConnectTimes = tryReConnectTimes;
}

int DBMessage::getTryReQueryTimes() const
{
    return m_tryReQueryTimes;
}

void DBMessage::setTryReQueryTimes(int tryReQueryTimes)
{
    m_tryReQueryTimes = tryReQueryTimes;
}
