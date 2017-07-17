TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

DEFINES +=USE_OPENCV CPU_ONLY

SOURCES += main.cpp
INCLUDEPATH += /home/ao/Downloads/Install-OpenCV/Ubuntu/2.4/caffe/include


LIBS += -lglog -lgflags -lprotobuf -lboost_system -lboost_thread -llmdb -lleveldb -lstdc++ -lcblas -latlas

LIBS += -lopencv_core -lopencv_imgproc -lopencv_highgui -lcaffe
