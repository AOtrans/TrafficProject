#ifndef TESTCLASS_H
#define TESTCLASS_H

#include "common.h"
#include <QObject>

class TestClass : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString m_name READ getName WRITE setName)
    Q_PROPERTY(QString m_id READ getId WRITE setId)
public:
    explicit TestClass(QObject *parent = 0);
    Q_INVOKABLE QString getId() const;
    Q_INVOKABLE void setId(const QString &value);

    Q_INVOKABLE QString getName() const;
    Q_INVOKABLE void setName(const QString &value);
    void debug();
private:
    QString m_name;
    QString m_id;
signals:

public slots:
};

#endif // TESTCLASS_H
