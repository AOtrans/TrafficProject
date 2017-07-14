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


INCLUDEPATH+=D:\Qt\opencvOnUse\include\opencv  \
             D:\Qt\opencvOnUse\include\opencv2 \
             D:\Qt\opencvOnUse\include         \
             D:\Qt\GnuWin32\include
INCLUDEPATH+=D:\Qt\Qt5.7.0\Boost1.62.0\include
INCLUDEPATH+=D:\armadillo7.6\include
INCLUDEPATH+=D:\mlpack2.1.1\include
INCLUDEPATH+=D:\OpenBLAS0.2.14\include
INCLUDEPATH+=D:\lapack3.7\include
INCLUDEPATH+=D:\armadillo7.6\include\armadillo_bits

LIBS += -L D:\Qt\opencvOnUse\lib\libopencv_*.a
LIBS += -L D:\Qt\GnuWin32\lib\lib*.a
LIBS += -L D:\armadillo7.6\lib\lib*.a
LIBS += -L D:\lapack3.7\lib\lib*.a
LIBS += -L D:\mlpack2.1.1\lib*.a
LIBS += -L D:\Qt\Qt5.7.0\Boost1.62.0\lib\lib*.a
LIBS += -L D:\OpenBLAS0.2.14\lib\libopenblas.dll.a
