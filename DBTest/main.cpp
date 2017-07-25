#include <QCoreApplication>
#include "dbmanager.h"
#include"common.h"
#include"testclass.h"
#include"settingutil.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    DBManager *m2=new DBManager(nullptr,new SettingUtil("/home/zg/traffic/MyKnowledageHolder/config.ini"));
    m2->connect();
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
