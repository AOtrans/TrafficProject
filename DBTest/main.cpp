#include <QCoreApplication>
#include "dbmanager.h"
#include"common.h"
#include"testclass.h"
#include"settingutil.h"





void parseXMLtoDB(DBManager *dbManager,QString xml)
{
    QString sql = "delete from group_info";
    dbManager->execQuery(sql);

    QDomDocument xmlContext;
    xmlContext.setContent(xml);
    QStringList sqlList;
    QDomElement root=xmlContext.documentElement();
    QDomNodeList targetList=root.elementsByTagName("area");
    for(int i=0; i<targetList.size();i++)
    {
        QDomNodeList valueList = targetList.at(i).toElement().childNodes();
        QString sql = "INSERT INTO group_info(id,name,level,parentId) VALUES(";
        QString temp = "";
        for(int j=0; j<valueList.size();j++)
        {
            temp += "'"+valueList.at(j).toElement().text()+"'";
            if(j!=valueList.size()-1)
                temp += ",";
        }
        sql += (temp + ")");
        qDebug()<<sql;
        sqlList.append(sql);
    }
    dbManager->totalCommand(sqlList);
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    DBManager *dbManager=new DBManager(nullptr,new SettingUtil("/home/zg/traffic/QtProject/DBTest/config.ini"));
    dbManager->connect();


    QFile file("/home/zg/traffic/QtProject/DBTest/dd.xml");
    file.open(QIODevice::ReadWrite);
    QTextStream ts(&file);


    parseXMLtoDB(dbManager, ts.readAll());
//    QVector<TestClass*> vec;
//    m2->lazyFun<TestClass>("select *from test",vec);
//    foreach(TestClass* i,vec)
//    {
//        i->debug();
//    }
//    QVector<QVariant> vec2;
//    vec2.append(QVariant("999"));
//    vec2.append(QVariant("666"));
//    m2->execQuery("insert into test(id,name)value(?,?)",vec2);
    return a.exec();
}
