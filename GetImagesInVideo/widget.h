#ifndef WIDGET_H
#define WIDGET_H

#include <QMessageBox>
#include <QFileDialog>
#include <QWidget>
#include <QTextStream>
#include "CarDetect/cardetector.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    vector<Rect> getCars(Mat &img);
    bool compareRect(Rect r,Rect rOther);
private slots:
    void on_pbChooseFile_clicked();

    void on_pbBegin_clicked();

private:
    Ui::Widget *ui;
    QString videoListFileName;
    QString currentDir;
    QFile successFile;
    QFile failFile;
    Detector *carDetector;
    float confidenceThreshold;
    vector<Rect> matchList;
};

#endif // WIDGET_H
