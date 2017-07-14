TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    iniutil.cpp \
    classifier.cpp \
    CarTracker/cartracker.cpp \
    CarFeature/carfeatureextract.cpp

INCLUDEPATH+=D:\Qt\opencvOnUse\include\opencv  \
             D:\Qt\opencvOnUse\include\opencv2 \
             D:\Qt\opencvOnUse\include

HEADERS += \
    common.h \
    iniutil.h \
    classifier.h \
    CarTracker/cartracker.h \
    CarFeature/carfeatureextract.h

DISTFILES += \
    config.ini
