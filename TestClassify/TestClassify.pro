QT += core
QT -= gui

CONFIG += c++11

TARGET = TestClassify
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    CarFeature/carfeatureextract.cpp \
    CarDetect/cardetector.cpp \
    settingutil.cpp \
    main.cpp \
    classifier.cpp

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

HEADERS += \
    CarFeature/carfeatureextract.h \
    CarDetect/cardetector.h \
    settingutil.h \
    common.h \
    classifier.h

DISTFILES += \
    config.ini

INCLUDEPATH += /usr/local/include/opencv \
               /usr/local/include/opencv2 \
               /home/zg/traffic/caffe-ssd/include

LIBS += -L/usr/local/lib  -L/home/zg/traffic/caffe-ssd/build/lib \
        -lswscale -lboost_system -lboost_thread -lglog -lgflags -lgomp -lpthread -lcaffe

# cuda
INCLUDEPATH += /usr/local/cuda/include
LIBS += -L/usr/local/cuda/lib64
LIBS += -lcudart -lcublas -lcurand -lcudnn
#opencv
LIBS +=-lopencv_calib3d -lopencv_contrib -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_gpu -lopencv_highgui \
-lopencv_imgproc -lopencv_legacy -lopencv_nonfree -lopencv_ocl -lopencv_photo -lopencv_stitching -lopencv_superres \
-lopencv_ts -lopencv_video -lopencv_videostab -lopencv_ml -lopencv_objdetect -lopencv_highgui -lopencv_core -lopencv_imgproc
