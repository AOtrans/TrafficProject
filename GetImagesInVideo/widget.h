#ifndef WIDGET_H
#define WIDGET_H

#include <QMessageBox>
#include <QFileDialog>
#include <QWidget>
#include <QTextStream>
#include "opencv2/opencv.hpp"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void on_pbChooseFile_clicked();

    void on_pbBegin_clicked();

private:
    Ui::Widget *ui;
    QString videoListFileName;
    QString currentDir;
    QFile successFile;
    QFile failFile;
};

#endif // WIDGET_H
