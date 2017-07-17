TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    iniutil.cpp \
    classifier.cpp \
    CarTracker/cartracker.cpp \
    CarFeature/carfeatureextract.cpp \
    CarDetect/cardetector.cpp \
    CarPlate/basicOperation.cpp \
    CarPlate/characterRecognition.cpp \
    CarPlate/characterSegmentation.cpp \
    CarPlate/colorImageEdgeDetect.cpp \
    CarPlate/getConnectedDomain.cpp \
    CarPlate/isPate.cpp \
    CarPlate/licensePlateLocation.cpp \
    CarPlate/Lprs.cpp \
    CarPlate/mysvmBaseLibsvm.cpp \
    CarPlate/plate.cpp \
    CarPlate/svm.cpp

HEADERS += \
    common.h \
    iniutil.h \
    classifier.h \
    CarTracker/cartracker.h \
    CarFeature/carfeatureextract.h \
    CarDetect/cardetector.h \
    CarPlate/basicOperation.h \
    CarPlate/characterRecognition.h \
    CarPlate/characterSegmentation.h \
    CarPlate/colorImageEdgeDetect.h \
    CarPlate/getConnectedDomain.h \
    CarPlate/isPate.h \
    CarPlate/licensePlateLocation.h \
    CarPlate/Lprs.h \
    CarPlate/mysvmBaseLibsvm.h \
    CarPlate/svm.h

DISTFILES += \
    config.ini

INCLUDEPATH += /usr/local/include/opencv \
               /usr/local/include/opencv2 \
               /home/zg/traffic/caffe-ssd/include \

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
