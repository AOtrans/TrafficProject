#include "dbmanager.h"
#include <QMutex>
QMutex dbMutex;
DBManager::DBManager(QObject *parent, DBMessage *DBmsg)
    :QObject(parent),m_DBmsg(DBmsg)
{
    m_query=nullptr;
}

DBManager::DBManager(QObject *parent, SettingUtil *util)
    :QObject(parent)
{
    m_query=nullptr;
    if(util!=nullptr)
    {
        qDebug()<<"DBInit";
        m_DBmsg=new DBMessage(util->getValue("DBConfig","dbmsName"),
                              util->getValue("DBConfig","hostName"),
                              util->getValue("DBConfig","port"),
                              util->getValue("DBConfig","dbName"),
                              util->getValue("DBConfig","account"),
                              util->getValue("DBConfig","password"),
                              util->getValue("DBConfig","tryReConnectTimes").toInt(),
                              util->getValue("DBConfig","tryReQueryTimes").toInt());
        qDebug()<<"DBInit done";
    }
}

bool DBManager::connect()
{
    if(m_db.isOpen())
    {
        DEBUG(m_DBmsg->getDbName()+"is already open");
        m_errorMsg=m_DBmsg->getDbName()+"is already open";
        return false;
    }
    if(m_DBmsg==nullptr)
    {
        DEBUG("has not set DBmsg");
        m_errorMsg="has not set DBmsg";
        return false;
    }
    m_db=QSqlDatabase::addDatabase(m_DBmsg->getDbmsName(),QUuid::createUuid().toString());
    m_db.setHostName(m_DBmsg->getHostName());
    m_db.setPort(m_DBmsg->getHostName().toInt());
    m_db.setDatabaseName(m_DBmsg->getDbName());
    m_db.setUserName(m_DBmsg->getAccount());
    m_db.setPassword(m_DBmsg->getPassword());
    m_query=new QSqlQuery(m_db);
    if(m_DBmsg->getDbmsName().contains(dbConfig::MYSQL))
    {
        if(!m_db.open())
        {
            m_errorMsg=m_db.lastError().text();
            DEBUG("-dbmsName="<<m_DBmsg->getDbName()<<"--returnMsg="<<m_errorMsg<<"-isOpen()="<<m_db.isOpen());
            return false;
        }
        else
        {
            m_errorMsg="noError";
            DEBUG("-dbmsName="<<m_DBmsg->getDbName()<<"--returnMsg="<<m_errorMsg<<"-isOpen()="<<m_db.isOpen());
            return true;
        }
    }
    return false;
    //do more type
}

void DBManager::disconnect()
{
    if(m_query!=nullptr)
    {
        delete m_query;
        m_query=nullptr;
    }
    m_db.close();
    DEBUG("-dbName"<<m_DBmsg->getDbmsName()<<" closed");
}

DBMessage *DBManager::DBmsg() const
{
    return m_DBmsg;
}

void DBManager::setDBmsg(DBMessage *DBmsg)
{
    if(DBmsg!=nullptr)
    {
        if(m_DBmsg!=nullptr)
            delete m_DBmsg;
        m_DBmsg = DBmsg;
    }
}

void DBManager::setDBmsg(SettingUtil *util)
{
    if(util!=nullptr)
    {
        if(m_DBmsg!=nullptr)
            delete m_DBmsg;
        m_DBmsg=new DBMessage(util->getValue(dbConfig::SegName,dbConfig::DBmsName),
                              util->getValue(dbConfig::SegName,dbConfig::HostName),
                              util->getValue(dbConfig::SegName,dbConfig::Port),
                              util->getValue(dbConfig::SegName,dbConfig::DBName),
                              util->getValue(dbConfig::SegName,dbConfig::Account),
                              util->getValue(dbConfig::SegName,dbConfig::Password),
                              util->getValue(dbConfig::SegName,dbConfig::TryReConnectTimes).toInt(),
                              util->getValue(dbConfig::SegName,dbConfig::TryReQueryTimes).toInt());
    }
}

bool DBManager::totalCommand(const QStringList &sqlList)
{
    if(sqlList.size()==0)
        return false;
    transaction();
    foreach(QString sql,sqlList)
    {
        if(!execQuery(sql))
        {
            rollback();
            commit();
            return false;
        }
    }
    commit();
    return true;
}

bool DBManager::transaction()
{
    while(m_query->isSelect()&&m_query->isActive()){usleep(500);};
    if(!m_db.transaction())
    {
        DEBUG("transaction Failed");
        return false;
    }
    else
    {
        DEBUG("transaction Successed");
        return true;
    }
}

bool DBManager::commit()
{
    while(m_query->isSelect()&&m_query->isActive()){usleep(500);};
    if(!m_db.commit())
    {
        DEBUG("commit Failed");
        return false;
    }
    else
    {
        DEBUG("commit Successed");
        return true;
    }
}

bool DBManager::rollback()
{
    while(m_query->isSelect()&&m_query->isActive()){usleep(500);};

    if(!m_db.rollback())
    {
        DEBUG("rollBack Failed");
        return false;
    }
    else
    {
        DEBUG("rollBack Successed");
        return true;
    }
}

bool DBManager::execQuery(QString sql)
{
    dbMutex.lock();
    if(!m_db.isOpen())
    {
        if(!tryReOpen())
        {
            DEBUG("Connect Fialed--"+m_db.lastError().text());
            m_errorMsg=m_db.lastError().text();
            dbMutex.unlock();
            return false;
        }
    }
    if(m_query==nullptr)
        m_query=new QSqlQuery(m_db);
    if(tryExec(sql))
    {
        dbMutex.unlock();
        return true;
    }
    else
    {
        m_errorMsg=m_query->lastError().text();
        dbMutex.unlock();
        return false;
    }
}

bool DBManager::execQuery(QString sql, const QVector<QVariant> &paraList)
{
    dbMutex.lock();
    if(!m_db.isOpen())
    {
        if(!tryReOpen())
        {
            DEBUG("Connect Fialed--"+m_db.lastError().text());
            m_errorMsg=m_db.lastError().text();
            dbMutex.unlock();
            return false;
        }
    }
    if(m_query==nullptr)
        m_query=new QSqlQuery(m_db);
    if(tryExec(sql,paraList))
    {
        dbMutex.unlock();
        return true;
    }
    else
    {
        m_errorMsg=m_query->lastError().text();
        dbMutex.unlock();
        return false;
    }
}

bool DBManager::tryReOpen()
{
    int i=0;
    while(!m_db.isOpen())
    {
        if(i==m_DBmsg->getTryReConnectTimes())
            return false;
        ++i;
        DEBUG(m_DBmsg->getDbName()+" is not open,tryTimes:"+QString::number(i)+",Error:"+m_db.lastError().text());
        m_db.open();
        usleep(100);
    }
    DEBUG(m_DBmsg->getDbName()+" open success");
    return true;
}

bool DBManager::tryExec(QString sql)
{
    int i=0;
    while(!m_query->exec(sql))
    {
        if(i==m_DBmsg->getTryReQueryTimes())
            return false;
        ++i;
        DEBUG(sql+"--ExecFailed,tryTimes:"+QString::number(i)+",Error:"+m_query->lastError().text());
        usleep(100);
    }
    DEBUG(sql+"--ExecSuccessed,AffectRows:"+QString::number(m_query->numRowsAffected()));
    return true;
}

bool DBManager::tryExec(QString sql, const QVector<QVariant> &paraList)
{
    int i=0;
    m_query->prepare(sql);
    for(int i=0;i<paraList.size();i++)
    {
        m_query->bindValue(i,paraList.at(i));
    }

    while(!m_query->exec())
    {
        if(i==m_DBmsg->getTryReQueryTimes())
        {
            DEBUG(sql+"--ExecFailed");
            return false;
        }
        ++i;
        DEBUG(sql+"--ExecFailed,tryTimes:"+QString::number(i)+",Error:"+m_query->lastError().text());
        usleep(500);
    }
    DEBUG(sql+"--ExecSuccessed,AffectRows:"+QString::number(m_query->numRowsAffected()));
    return true;
}

int DBManager::rowCount()
{
    return m_query->size();
}

int DBManager::colCount()
{
    return m_query->record().count();
}

QVariant DBManager::getData(int colIndex)
{
    if(colIndex>=colCount())
        return QVariant();
    return m_query->value(colIndex);
}

QVariant DBManager::getData(QString name)
{
    return m_query->value(name);
}

bool DBManager::next()
{
    return m_query->next();
}

bool DBManager::previous()
{
    return m_query->previous();
}

bool DBManager::seek(int index, bool relative)
{
    return m_query->seek(index,relative);
}

void DBManager::finish()
{
    m_query->finish();
}

