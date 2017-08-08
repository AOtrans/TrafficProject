#-------------------------------------------------
#
# Project created by QtCreator 2017-03-07T12:45:24
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TxtUtil
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
INCLUDEPATH += /usr/local/include/opencv \
               /usr/local/include/opencv2 \

LIBS += -L/usr/local/lib  -L/home/zg/traffic/caffe-ssd/build/lib \
        -lopencv_highgui -lopencv_core -lopencv_imgproc

