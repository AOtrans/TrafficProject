TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt



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

INCLUDEPATH += /usr/local/include/opencv \
               /usr/local/include/opencv2 \

LIBS += -L/usr/local/lib  -L/home/zg/traffic/caffe-ssd/build/lib \
        -lopencv_highgui -lopencv_core -lopencv_imgproc \
        -lswscale -lboost_system -lboost_thread -lglog -lgflags -lgomp -lpthread -lcaffe -lboost_program_options -lboost_filesystem
