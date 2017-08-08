#include "Lprs.h"
#include <iostream>
#include <stdio.h>
string getLicense(string dealPathStr,string plateSvmTrainModel,string enTrainModel,string chTrainModel)
{
	Lprs plateSystem(plateSvmTrainModel,enTrainModel,chTrainModel);

	Mat plateImage = imread(dealPathStr);

   string plateNumber = plateSystem.prosess(plateImage);

   return plateNumber;
}

