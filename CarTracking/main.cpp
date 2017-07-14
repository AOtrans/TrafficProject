#include "CarTracker/cartracker.h"

int main(int argc, char *argv[])
{
    CarTracker *c=CarTracker::getInstence("./config.ini");
    cout<<c->carTrack("D:/work/ppt/xx.mp4","car","white","","")<<endl;
    return 0;
}
