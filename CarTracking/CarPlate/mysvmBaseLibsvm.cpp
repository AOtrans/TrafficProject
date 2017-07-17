#include "mysvmBaseLibsvm.h"

mysvmBaseLibsvm::mysvmBaseLibsvm(vector<string> &path,string type)
{
	//svm_parameter 参数初始化 RBF
	param.svm_type = C_SVC;
	param.kernel_type = RBF;
	param.degree = 3;
	param.gamma = 0.5;
	param.coef0 = 0;
	param.nu = 0.5;
	param.cache_size = 40;
	param.C = 500;
	param.eps = 0.001;
	param.p = 0.1;
	param.shrinking = 1;
	param.probability = 0;
	param.nr_weight = 0;
	param.weight = NULL;
	param.weight_label =NULL;

	//其他参数初始化
	whichContruct = 1;

	int sampleNumbers = 0;
	int pathSize = path.size();
	for(int i = 0;i < pathSize;i++)
	{
		int strLength = path[i].length();
		if(path[i].substr(strLength-3,3) == "jpg")
			sampleNumbers++;
	}

	problem.l = sampleNumbers;
	problem.y = new double [sampleNumbers];
	problem.x = new svm_node* [sampleNumbers];

	this->type = type;

	classify(path);                                       //得到所有特征
}

mysvmBaseLibsvm::mysvmBaseLibsvm(string type)
{
	whichContruct = 2;
	this->type = type;
	//loadModel();	
}

mysvmBaseLibsvm::mysvmBaseLibsvm()
{

}

mysvmBaseLibsvm::~mysvmBaseLibsvm()
{
	if(whichContruct == 1)
	{
		delete [] problem.y;
		delete [] problem.x;
		svm_free_and_destroy_model(&model);
	}
}

void mysvmBaseLibsvm::saveModel()                        //保存训练好的参数
{
	if(type == "ch")
	{
		if(svm_save_model(chTrainModel.c_str(),model) == 0)
			cout<<"model ch save successful !"<<endl;
		else
			cout<<"model ch save failed !"<<endl;
	}
	else if(type == "en")
	{
		if(svm_save_model(enTrainModel.c_str(),model) == 0)
			cout<<"model en save successful !"<<endl;
		else
			cout<<"model  en save failed !"<<endl;
	}
	
}

void mysvmBaseLibsvm::loadModel()                        //加载训练好的参数
{
	if(type == "ch")
	{
		model = svm_load_model(chTrainModel.c_str());
	}
	else if(type == "en")
	{
		model = svm_load_model(enTrainModel.c_str());
	}

	if(model)
		cout<<"load "<<this->type<<" model successful !"<<endl;
	else
		cout<<"load "<<this->type<<" model failed !"<<endl;
}

int mysvmBaseLibsvm::sumMatValue(const Mat& image)
{
	int sumValue = 0; 
	int r = image.rows; 
	int c = image.cols; 
	if (image.isContinuous()) 
	{ 
		c = r*c; 
		r = 1;    
	} 
	for (int i = 0; i < r; i++) 
	{ 
		const uchar* linePtr = image.ptr<uchar>(i); 
		for (int j = 0; j < c; j++) 
		{ 
			sumValue += linePtr[j]; 
		} 
	} 
	return sumValue; 
}

void mysvmBaseLibsvm::getFeature(Mat &plate)                 //获得特征
{
	feature.clear();
	int i,j;
	Mat image = plate.clone();
	int rows = image.rows;
	int cols = image.cols;
	float totleValue = sumMatValue(image); 
	for (i = 0; i < rows; i++) 
	{ 
		uchar* ptr = image.ptr<uchar>(i);
        for (j = 0; j < cols;j++)
		{ 			
			feature.push_back(ptr[j]*1.0 / totleValue); 
		} 
	} 
}

void mysvmBaseLibsvm::myLibsvmTrain()                    //训练
{
	if(!svm_check_parameter(&problem,&param))
		cout<<"svm_parameter is right !"<<endl;
	model = svm_train(&problem,&param);
	saveModel();
}
double mysvmBaseLibsvm::myLibsvmPredict(Mat &plate)       //预测
{
	//loadModel();
	getFeature(plate);
	int featureDimension = feature.size();
	svm_node* node = new svm_node[featureDimension+1];
	int i;
	for(i = 0;i < featureDimension;i++)
	{
		node[i].index = i;
		node[i].value = feature[i];
	}
	node[i].index = -1;

	double result = svm_predict(model,node);

	delete []node;

	return result;
}

void mysvmBaseLibsvm::classify(vector<string> &path)           //根据路径判断该分类，确定目标输出
{
	int pathSize = path.size();
	int i,j;

	for(i = 0;i < pathSize;i++)
	{
		int strLength = path[i].length();

		if(path[i].substr(strLength-3,3) != "jpg")
			continue;

		Mat plate = imread(path[i],0);
		getFeature(plate);                                     //获得特征

		for(j = strLength-1;j > -1;j--)
		{
			if(path[i][j] == '\\')
				break;
		}

		int label = (path[i][j-2]-48)*10 + path[i][j-1]-48;      //目标输出,文件夹数字命名 00-99

		int featureDimension = feature.size();                   //特征维数，可改进方向，忽略为零的特征，着存储可看成稀疏矩阵，这里未实现
		static int sampleIndex = 0;                              //样本索引
		//int index = 0;                                           //特征维数索引
		svm_node* features = new svm_node[featureDimension+1];   //特征变量，-1结尾，libsvm规定

		problem.y[sampleIndex] = label;                         //赋值目标输出
		for(j = 0;j < featureDimension;j++)
		{
			features[j].index = j;
			features[j].value = feature[j];
		}
		features[j].index = -1;

		problem.x[sampleIndex++] = features;                   
	}
}

void mysvmBaseLibsvm::setEnTrainModelPath(string enTrainModel)
{
	this->enTrainModel = enTrainModel;
}

void mysvmBaseLibsvm::setChTrainModelPath(string chTrainModel)
{
	this->chTrainModel = chTrainModel;
}

void mysvmBaseLibsvm::setType(string type)
{
	this->type = type;
}

void mysvmBaseLibsvm::setFreeModel()
{
	svm_free_and_destroy_model(&model);
}
