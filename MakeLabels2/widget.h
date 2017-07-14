#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QButtonGroup>
#include <QFileDialog>
#include <IniUtil.h>
#include <QDebug>
#include <QPixmap>
#include <QImage>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    void init();
    void changeImg(QString filePath);
    void closeEvent(QCloseEvent *event);
    void saveLabel();
    void checkLabel();
private slots:
    void on_pbChooseDir_clicked();

    void on_pbPre_clicked();

    void on_pbNext_clicked();

    void on_pbDelete_clicked();

private:
    Ui::Widget *ui;
    QButtonGroup *group1;
    QButtonGroup *group2;
    QButtonGroup *group3;
    IniUtil *iniUtil;
    int currentIndex;
    QString currentFile;
    QFileInfoList ls;
    QString dirPath;
};

#endif // WIDGET_H
