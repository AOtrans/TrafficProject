#-------------------------------------------------
#
# Project created by QtCreator 2017-05-09T12:46:22
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GetImagesInVideo
TEMPLATE = app

INCLUDEPATH+=D:\Qt\opencvOnUse\include\opencv  \
             D:\Qt\opencvOnUse\include\opencv2 \
             D:\Qt\opencvOnUse\include

LIBS += -L D:\Qt\opencvOnUse\lib\libopencv_*.a

SOURCES += main.cpp\
        widget.cpp

HEADERS  += widget.h

FORMS    += widget.ui
