TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH+=D:/Qt/opencvOnUse/include/opencv  \
             D:/Qt/opencvOnUse/include/opencv2 \
             D:/Qt/opencvOnUse/include
INCLUDEPATH+=D:/Qt/Qt5.7.0/Boost1.62.0/include
LIBS += D:/Qt/Qt5.7.0/Boost1.62.0/lib/libboost_*.a
LIBS += D:/Qt/opencvOnUse/lib/libopencv_*.a

SOURCES += \
    DataAugmentation.cpp \
    main.cpp \
    RandomRotation.cpp \
    Util.cpp

DISTFILES += \
    annotation.txt \
    config.txt

HEADERS += \
    DataAugmentation.h \
    RandomRotation.h \
    Util.h
