#include "testclass.h"

TestClass::TestClass(QObject *parent) : QObject(parent)
{

}

QString TestClass::getId() const
{
    return m_id;
}

void TestClass::setId(const QString &value)
{
    m_id = value;
}

QString TestClass::getName() const
{
    return m_name;
}

void TestClass::setName(const QString &value)
{
    m_name = value;
}

void TestClass::debug()
{
    qDebug()<<m_name<<m_id;
}
