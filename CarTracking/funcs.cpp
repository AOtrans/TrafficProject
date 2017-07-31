#include "funcs.h"
#include <QMutex>
extern QMutex mutex;

string getPlate(const char *path)
{
    CarTracker *c=CarTracker::getInstence();
    mutex.lock();
    Mat img =imread(path);
    string plate=c->getPlate(img);
    mutex.unlock();
    return plate;
}
