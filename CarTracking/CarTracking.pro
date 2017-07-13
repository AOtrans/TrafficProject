TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

INCLUDEPATH+=D:\Qt\opencvOnUse\include\opencv  \
             D:\Qt\opencvOnUse\include\opencv2 \
             D:\Qt\opencvOnUse\include
LIBS += -L D:\Qt\opencvOnUse\lib\libopencv_*.a

HEADERS += \
    common.h
