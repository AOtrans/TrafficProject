#-------------------------------------------------
#
# Project created by QtCreator 2017-05-09T12:46:22
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GetImagesInVideo
TEMPLATE = app

INCLUDEPATH += /usr/local/include/opencv \
               /usr/local/include/opencv2 \
               /home/zg/traffic/caffe-ssd/include \

LIBS += -L/usr/local/lib  -L/home/zg/traffic/caffe-ssd/build/lib \
        -lopencv_highgui -lopencv_core -lopencv_imgproc \
        -lswscale -lboost_system -lboost_thread -lglog -lgflags -lgomp -lpthread -lcaffe

# cuda
INCLUDEPATH += /usr/local/cuda/include
LIBS += -L/usr/local/cuda/lib64
LIBS += -lcudart -lcublas -lcurand -lcudnn


SOURCES += main.cpp\
        widget.cpp \
    CarDetect/cardetector.cpp

HEADERS  += widget.h \
    CarDetect/cardetector.h \
    common.h

FORMS    += widget.ui
