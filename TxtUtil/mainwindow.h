#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileDialog>
#include <QDir>
#include <QDebug>
#include <QTextStream>
#include <QFile>
#include <QString>
#include <QMainWindow>
#include <opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/ml/ml.hpp>
#include "highgui.h"
#include "cv.h"
#include "cxcore.hpp"
#include "iostream"
#include "opencv2/opencv.hpp"
#include "opencv2/nonfree/nonfree.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void doNormal(QDir &dir);
    void dosubDir(QString prefix,int label,QMap<QString,int> &typeMap,QMap<QString,int> &nameMap,QTextStream &ts,QDir &subDir,bool ifDo=true,bool ifDoSub=true);
    void dosubsubDir(QString prefix,int label,int label2,QMap<QString,int> &nameMap,QTextStream &ts,QDir &subsubDir,bool ifDo=true);
    void doMultiTxt(QString prefix,int label1,int label2,int label3,QTextStream& ts,QDir &subsubsubDir);
private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_normalized_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pbFileTxt_clicked();

private:
    Ui::MainWindow *ui;
    QString fileName;
    QString dirName;
};

#endif // MAINWINDOW_H
