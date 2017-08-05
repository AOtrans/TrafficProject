#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QMutex>
#include <QTime>
#include <QUuid>
#include "dbcommon.h"
#include "dbmessage.h"
#include "../settingutil.h"

typedef void (*lazy)(int index, QString name, const QVariant &value);

class DBManager : public QObject
{
    Q_OBJECT
public:
    explicit DBManager(QObject *parent = 0, DBMessage *DBmsg = nullptr);
    explicit DBManager(QObject *parent = 0, SettingUtil *util = nullptr);

    bool connect();
    void disconnect();
    DBMessage *DBmsg() const;
    void setDBmsg(DBMessage *DBmsg);
    void setDBmsg(SettingUtil *util);

    bool totalCommand(const QStringList &sqlList);
    bool transaction();
    bool commit();
    bool rollback();
    bool execQuery(QString sql);
    bool execQuery(QString sql, const QVector<QVariant> &paraList);
    bool tryReOpen();
    bool tryExec(QString sql);
    bool tryExec(QString sql, const QVector<QVariant> &paraList);

    int rowCount();
    int colCount();
    QVariant getData(int colIndex);
    QVariant getData(QString name);
    bool next();
    bool previous();
    bool seek(int index, bool relative = false);

    template<typename T>
    bool lazyFun(QString sql, QVector<T*> &vec)
    {

        if(!execQuery(sql))
            return false;

        while(!m_query->isActive()){}

        vec.clear();
        const QSqlRecord &record = m_query->record();

        int index = 0;

        while(m_query->next())
        {
            vec.append(new T());

            for(int i = 0; i < record.count(); i++)
            {
                vec[index]->setProperty(("m_" + record.fieldName(i)).toStdString().c_str(), m_query->value(i));
            }

            ++index;
        }

        return true;
    }

    void finish();
private:
    DBMessage *m_DBmsg;
    QSqlDatabase m_db;
    QSqlQuery *m_query;
    QString m_errorMsg;
    //QMutex m_mutex;
signals:

public slots:
};

#endif // DBMANAGER_H
