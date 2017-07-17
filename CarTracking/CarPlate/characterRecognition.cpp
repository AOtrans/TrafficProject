#include "characterRecognition.h"

string table[] = {"0","1","2","3","4","5","6","7","8","9","A","B","C","D","E","F","G","H","J","K","L","M","N","P","Q","R","S","T","U","V","W","X","Y","Z","学"};
string tableCH[] = {"川","鄂","赣","甘","贵","桂","黑","沪","冀","津","京","吉","辽","鲁","蒙","闽","宁","青","琼","陕","苏","晋","皖","湘","新","豫","渝","粤","云","藏","浙"};

void characterRecognition::setEnTrainModelPath(string enTrainModel)
{
	this->enTrainModel = enTrainModel;
}

void characterRecognition::setChTrainModelPath(string chTrainModel)
{
	this->chTrainModel = chTrainModel;
}

void characterRecognition::setCharRecognitionParam()
{
	mySvmCH.setType("ch");
	mySvmEN.setType("en");

	mySvmCH.setChTrainModelPath(chTrainModel);
	mySvmEN.setEnTrainModelPath(enTrainModel);

	mySvmCH.loadModel();
	mySvmEN.loadModel();
}

string characterRecognition::process(vector<Mat> &plateChar)
{
	int plateCharLength = plateChar.size();
	int i;
	string licensePlateNumber;

	for(i = 0;i < plateCharLength;i++)
	{
		int result;
		if(i == 0)
		{
			result = (int)mySvmCH.myLibsvmPredict(plateChar[i]);   //这样直接转换是否会出现问题？？？？？？？比如3.9，是取3还是4呢，有待考证源码实现（libsvm）        
            licensePlateNumber = tableCH[result];
		}
		else
		{
			result = (int)mySvmEN.myLibsvmPredict(plateChar[i]);   //这样直接转换是否会出现问题？？？？？？？比如3.9，是取3还是4呢          
			licensePlateNumber += table[result];
		}
	}
	return licensePlateNumber;
}

void characterRecognition::setFreeEN_CHModel()
{
	mySvmCH.setFreeModel();
	mySvmEN.setFreeModel();
}

