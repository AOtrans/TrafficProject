#include "isPate.h"

isPate::isPate()
{
	SVM_params.kernel_type = CvSVM::LINEAR;
}

void isPate::getFeture(vector<string> &path)
{
	int size = path.size();
	int i,j;

	for(i = 0;i < size;i++)
	{
		string test = path[i].substr(path[i].length()-3,3);
		if(path[i].substr(path[i].length()-3,3) != "jpg")
			continue;

		//设置标签
		string pateName;
		bool mark = false;
		for(j = path[i].length()-1;j > 0;j--)
		{
			while(path[i][j--] != '\\');
			while(path[i][j] != '\\')
			{
				pateName += path[i][j];
				j--;
			}
			if(pateName == "SP")
			{
				mark = true;
				break;
			}
			else
			{
				mark = false;
				break;
			}
		}
		if(mark)
		{
			Mat pLable = Mat::zeros(1,1,CV_32FC1);
			pLable.at<float>(0,0) = 1.0;
			label.push_back(pLable);
		}
		else
		{
			Mat pLable = Mat::zeros(1,1,CV_32FC1);
			pLable.at<float>(0,0) = -1.0;
			label.push_back(pLable);
		}

		Mat pate = imread(path[i],0);

		resize(pate,pate,Size(128,64));
		HOGDescriptor myHOG(Size(128,64),Size(16,16),Size(8,8),Size(8,8),9);
		vector<float> HOGFeatrue;
		myHOG.compute(pate,HOGFeatrue,Size(128,64),Size(0,0));
		Mat pate1(HOGFeatrue);
		Mat pate2 = pate1.reshape(1,1);

		feture.push_back(pate2);                  //变量特征
	}
}

void isPate::svmTrain()
{
	classifier.train(feture,label ,Mat(),Mat(),SVM_params);
	saveSvm();
}

int isPate::svmPredict(Mat &pate)
{
	fstream xmlOpen;
	xmlOpen.open(plateSvmTrainModel,ios::binary | ios::in);
	if(!xmlOpen)
	{
		cout<<"open xml fail!"<<endl;
		exit(0);
	}
	char test;
	xmlOpen.get(&test,1);
	if(xmlOpen.eof())
	{
		cout<<"xml is empty!"<<endl;
		xmlOpen.close();
		exit(0);
	}
	xmlOpen.close();

	readSvm();

	Mat pateChange = pate.clone();

	resize(pateChange,pateChange,Size(128,64));
	HOGDescriptor myHOG(Size(128,64),Size(16,16),Size(8,8),Size(8,8),9);
	vector<float> HOGFeatrue;
	myHOG.compute(pateChange,HOGFeatrue,Size(128,64),Size(0,0));
	Mat pate1(HOGFeatrue);
	Mat pate2 = pate1.reshape(1,1);

	int pre = (int)classifier.predict(pate2);

	return pre;
}

void isPate::saveSvm()
{
	classifier.save(plateSvmTrainModel.c_str());
}

void isPate::readSvm()
{
	classifier.load(plateSvmTrainModel.c_str());
}

void isPate::setTrainModelPath(string plateSvmTrainModel)
{
	this->plateSvmTrainModel = plateSvmTrainModel;
}
