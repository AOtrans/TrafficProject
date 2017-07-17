#include "Lprs.h"

Lprs::Lprs(string plateSvmTrainModel,string enTrainModel,string chTrainModel)
{
    lpl.setPlateSvmTrainModel(plateSvmTrainModel);
    lpl.setIsPateParam();

    cr.setChTrainModelPath(chTrainModel);
    cr.setEnTrainModelPath(enTrainModel);
    cr.setCharRecognitionParam();
}

Lprs::~Lprs()
{
    cr.setFreeEN_CHModel();
}

string Lprs::prosess(Mat image)
{
    vector<Mat> pate;
    string plateNumber;

    lpl.process(image,pate);

    if(pate.size() < 1)
        return "1";
    int pateLength = pate.size();
    for(int j = 0;j < pateLength;j++)
    {
        //??????????
        vector<Mat> pateChar;
        if(!cs.process(pate[j],pateChar))
            return "2";

        //??????????
        plateNumber = cr.process(pateChar);
    }
    return plateNumber;
}
