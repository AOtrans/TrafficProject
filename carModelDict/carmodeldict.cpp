#include "carmodeldict.h"
//#include <QDir>
//#include <vector>
using namespace caffe;  // NOLINT(build/namespaces)
using namespace std;

/* Pair (label, confidence) representing a prediction. */
typedef std::pair<string, float> Prediction;

class Classifier {
public:
    Classifier(const string& model_file,
               const string& trained_file,
               const string& mean_file,
               const string& label_file);

    std::vector<Prediction> Classify(const cv::Mat& img, int N = 5);

private:
    void SetMean(const string& mean_file);

    std::vector<float> Predict(const cv::Mat& img);

    void WrapInputLayer(std::vector<cv::Mat>* input_channels);

    void Preprocess(const cv::Mat& img,
                    std::vector<cv::Mat>* input_channels);

private:
    boost::shared_ptr<Net<float> > net_;
    cv::Size input_geometry_;
    int num_channels_;
    cv::Mat mean_;
    std::vector<string> labels_;
};

Classifier::Classifier(const string& model_file,
                       const string& trained_file,
                       const string& mean_file,
                       const string& label_file) {
#ifdef CPU_ONLY
    Caffe::set_mode(Caffe::CPU);
#else
    Caffe::set_mode(Caffe::GPU);
#endif

    /* Load the network. */
    net_.reset(new Net<float>(model_file, TEST));
    net_->CopyTrainedLayersFrom(trained_file);

    CHECK_EQ(net_->num_inputs(), 1) << "Network should have exactly one input.";
    CHECK_EQ(net_->num_outputs(), 1) << "Network should have exactly one output.";

    Blob<float>* input_layer = net_->input_blobs()[0];
    num_channels_ = input_layer->channels();
    CHECK(num_channels_ == 3 || num_channels_ == 1)
            << "Input layer should have 1 or 3 channels.";
    input_geometry_ = cv::Size(input_layer->width(), input_layer->height());

    /* Load the binaryproto mean file. */
    SetMean(mean_file);

    /* Load labels. */
    std::ifstream labels(label_file.c_str());
    CHECK(labels) << "Unable to open labels file " << label_file;
    string line;
    while (std::getline(labels, line))
        labels_.push_back(string(line));

    Blob<float>* output_layer = net_->output_blobs()[0];
    CHECK_EQ(labels_.size(), output_layer->channels())
            << "Number of labels is different from the output layer dimension.";
}

static bool PairCompare(const std::pair<float, int>& lhs,
                        const std::pair<float, int>& rhs) {
    return lhs.first > rhs.first;
}

/* Return the indices of the top N values of vector v. */
static std::vector<int> Argmax(const std::vector<float>& v, int N) {
    std::vector<std::pair<float, int> > pairs;
    for (size_t i = 0; i < v.size(); ++i)
        pairs.push_back(std::make_pair(v[i], i));
    std::partial_sort(pairs.begin(), pairs.begin() + N, pairs.end(), PairCompare);

    std::vector<int> result;
    for (int i = 0; i < N; ++i)
        result.push_back(pairs[i].second);
    return result;
}

/* Return the top N predictions. */
std::vector<Prediction> Classifier::Classify(const cv::Mat& img, int N) {
    std::vector<float> output = Predict(img);

    N = std::min<int>(labels_.size(), N);
    std::vector<int> maxN = Argmax(output, N);
    std::vector<Prediction> predictions;
    for (int i = 0; i < N; ++i) {
        int idx = maxN[i];
        predictions.push_back(std::make_pair(labels_[idx], output[idx]));
    }

    return predictions;
}

/* Load the mean file in binaryproto format. */
void Classifier::SetMean(const string& mean_file) {
    BlobProto blob_proto;
    ReadProtoFromBinaryFileOrDie(mean_file.c_str(), &blob_proto);

    /* Convert from BlobProto to Blob<float> */
    Blob<float> mean_blob;
    mean_blob.FromProto(blob_proto);
    CHECK_EQ(mean_blob.channels(), num_channels_)
            << "Number of channels of mean file doesn't match input layer.";

    /* The format of the mean file is planar 32-bit float BGR or grayscale. */
    std::vector<cv::Mat> channels;
    float* data = mean_blob.mutable_cpu_data();
    for (int i = 0; i < num_channels_; ++i) {
        /* Extract an individual channel. */
        cv::Mat channel(mean_blob.height(), mean_blob.width(), CV_32FC1, data);
        channels.push_back(channel);
        data += mean_blob.height() * mean_blob.width();
    }

    /* Merge the separate channels into a single image. */
    cv::Mat mean;
    cv::merge(channels, mean);

    /* Compute the global mean pixel value and create a mean image
   * filled with this value. */
    cv::Scalar channel_mean = cv::mean(mean);
    mean_ = cv::Mat(input_geometry_, mean.type(), channel_mean);
}

std::vector<float> Classifier::Predict(const cv::Mat& img) {
    Blob<float>* input_layer = net_->input_blobs()[0];
    input_layer->Reshape(1, num_channels_,
                         input_geometry_.height, input_geometry_.width);
    /* Forward dimension change to all layers. */
    net_->Reshape();

    std::vector<cv::Mat> input_channels;
    WrapInputLayer(&input_channels);

    Preprocess(img, &input_channels);

    net_->Forward();

    /* Copy the output layer to a std::vector */
    Blob<float>* output_layer = net_->output_blobs()[0];
    const float* begin = output_layer->cpu_data();
    const float* end = begin + output_layer->channels();
    return std::vector<float>(begin, end);
}

/* Wrap the input layer of the network in separate cv::Mat objects
 * (one per channel). This way we save one memcpy operation and we
 * don't need to rely on cudaMemcpy2D. The last preprocessing
 * operation will write the separate channels directly to the input
 * layer. */
void Classifier::WrapInputLayer(std::vector<cv::Mat>* input_channels) {
    Blob<float>* input_layer = net_->input_blobs()[0];

    int width = input_layer->width();
    int height = input_layer->height();
    float* input_data = input_layer->mutable_cpu_data();
    for (int i = 0; i < input_layer->channels(); ++i) {
        cv::Mat channel(height, width, CV_32FC1, input_data);
        input_channels->push_back(channel);
        input_data += width * height;
    }
}

void Classifier::Preprocess(const cv::Mat& img,
                            std::vector<cv::Mat>* input_channels) {
    /* Convert the input image to the input image format of the network. */
    cv::Mat sample;
    if (img.channels() == 3 && num_channels_ == 1)
        cv::cvtColor(img, sample, cv::COLOR_BGR2GRAY);
    else if (img.channels() == 4 && num_channels_ == 1)
        cv::cvtColor(img, sample, cv::COLOR_BGRA2GRAY);
    else if (img.channels() == 4 && num_channels_ == 3)
        cv::cvtColor(img, sample, cv::COLOR_BGRA2BGR);
    else if (img.channels() == 1 && num_channels_ == 3)
        cv::cvtColor(img, sample, cv::COLOR_GRAY2BGR);
    else
        sample = img;

    cv::Mat sample_resized;
    if (sample.size() != input_geometry_)
        cv::resize(sample, sample_resized, input_geometry_);
    else
        sample_resized = sample;

    cv::Mat sample_float;
    if (num_channels_ == 3)
        sample_resized.convertTo(sample_float, CV_32FC3);
    else
        sample_resized.convertTo(sample_float, CV_32FC1);

    cv::Mat sample_normalized;
    cv::subtract(sample_float, mean_, sample_normalized);

    /* This operation will write the separate BGR planes directly to the
   * input layer of the network because it is wrapped by the cv::Mat
   * objects in input_channels. */
    cv::split(sample_normalized, *input_channels);

    CHECK(reinterpret_cast<float*>(input_channels->at(0).data)
          == net_->input_blobs()[0]->cpu_data())
            << "Input channels are not wrapping the input layer of the network.";
}

// void testResult()
// {
//     //    if (argc != 6) {
//     //        std::cerr << "Usage: " << argv[0]
//     //                  << " deploy.prototxt network.caffemodel"
//     //                  << " mean.binaryproto labels.txt img.jpg" << std::endl;
//     //        return 1;
//     //    }
//     //  string model_file   = argv[1];
//     //  string trained_file = argv[2];
//     //  string mean_file    = argv[3];
//     //  string label_file   = argv[4];
//     string model_file   = "googleCar/deploy.prototxt";
//     string trained_file = "googleCar/model_iter_4000.caffemodel";
//     string mean_file    = "googleCar/imagenet_mean.binaryproto";
//     string label_file   = "googleCar/labels.txt";
//     Classifier classifier(model_file, trained_file, mean_file, label_file);

//     //  string file = argv[5];
//     //QString fileDir="/home/ao/Downloads/Install-OpenCV/Ubuntu/2.4/caffe/data/myself/train/samples/middle/suv";
//     //QString fileDir="/home/ao/Downloads/Install-OpenCV/Ubuntu/2.4/caffe/data/makeData/samples/bigCoach";
//     QStringList ls;
//     ls<<"/home/ao/Downloads/Install-OpenCV/Ubuntu/2.4/caffe/data/makeData/samples/bigCoachTest" \
//      <<"/home/ao/Downloads/Install-OpenCV/Ubuntu/2.4/caffe/data/makeData/samples/bigTruckTest" \
//     <<"/home/ao/Downloads/Install-OpenCV/Ubuntu/2.4/caffe/data/makeData/samples/carTest" \
//     <<"/home/ao/Downloads/Install-OpenCV/Ubuntu/2.4/caffe/data/makeData/samples/mpvTest" \
//     <<"/home/ao/Downloads/Install-OpenCV/Ubuntu/2.4/caffe/data/makeData/samples/suvTest";
//     //QString fileDir="./cars";
//     int count=0;
//     int pos=0;
//     int flag=0;
//     foreach(QString fileDir,ls)
//     {
//         QDir dir(fileDir);
//         QStringList filter;
//         filter<<"*.jpg"<<"*.jpeg"<<"*.png";
//         foreach(QFileInfo fileInfo,dir.entryInfoList(filter))
//         {
//             //cv::waitKey();
//             count++;
//             std::cout << "---------- Prediction for "
//                       << fileInfo.fileName().toStdString() << " ----------" << std::endl;

//             cv::Mat img = cv::imread(fileInfo.filePath().toStdString(), -1);
//             CHECK(!img.empty()) << "Unable to decode image " << fileInfo.fileName().toStdString();
//             std::vector<Prediction> predictions = classifier.Classify(img);


//             putText(img,predictions[0].first,cv::Point( img.rows/4,img.cols/4),CV_FONT_HERSHEY_COMPLEX, 1, cv::Scalar(0, 0, 255) );
//             cv::imshow("window",img);
//             cv::waitKey(50);
//             /* Print the top N predictions. */
//             for (size_t i = 0; i < predictions.size(); ++i) {
//                 Prediction p = predictions[i];
//                 std::cout << std::fixed << std::setprecision(4) << p.second << " - \""
//                           << p.first << "\"" << std::endl;
//             }

//             QString temp=predictions[0].first.data();
//             temp=temp.mid(0,1);
//             //std::cout<<temp.toStdString()<<std::endl;
//             if(flag==temp.toInt())
//             {
//                 pos++;
//             }
//         }
//         flag++;
//     }
//     std::cout<<count<<std::endl;
//     std::cout<<"accuracy:"<<float(pos)/float(count)<<std::endl;
// }

CarModelDict::CarModelDict(const char *configFilePath):classifier(NULL)
{
    IniUtil util;
    if(util.OpenFile(configFilePath,"r")!=INI_SUCCESS)
    {
        std::cout<<"openConfigFileError"<<std::endl;
    }
    else
    {
        string model_file   = string(util.GetStr("CarModel","modelFilePath"));
        string trained_file = string(util.GetStr("CarModel","trainedFilePath"));
        string mean_file    = string(util.GetStr("CarModel","meanFilePath"));
        string label_file   = string(util.GetStr("CarModel","labelFilePath"));;
        classifier=new Classifier(model_file, trained_file, mean_file, label_file);
        util.CloseFile();
    }
}

CarModelDict::CarModelDict():classifier(NULL)
{

}

CarModelDict::~CarModelDict()
{
    if(classifier!=NULL)
        delete classifier;
}

bool CarModelDict::reInit(const char *configFilePath)
{
    IniUtil util;
    if(util.OpenFile(configFilePath,"r")!=INI_SUCCESS)
    {
        std::cout<<"openConfigFileError"<<std::endl;
        return false;
    }
    else
    {
        if(classifier!=NULL)
            delete classifier;
        string model_file   = string(util.GetStr("CarModel","modelFilePath"));
        string trained_file = string(util.GetStr("CarModel","trainedFilePath"));
        string mean_file    = string(util.GetStr("CarModel","meanFilePath"));
        string label_file   = string(util.GetStr("CarModel","labelFilePath"));;
        classifier=new Classifier(model_file, trained_file, mean_file, label_file);
        util.CloseFile();
        return true;
    }
}

bool CarModelDict::checkImageFormat(cv::Mat &img)
{
    if(img.empty())
    {
        std::cout<<"image is empty"<<std::endl;
        return false;
    }
    if(img.channels()!=3)
    {
        std::cout<<"current channel:"<<img.channels()<<" resize to standard channel 3"<<std::endl;
        cv::cvtColor(img,img,CV_GRAY2BGR);
    }
    if(img.rows!=256||img.cols!=256)
    {
        std::cout<<"resize to standard scale 256*256"<<std::endl;
        cv::resize(img,img,cv::Size(256,256));
    }
    return true;
}

string CarModelDict::singleImageCarModelDict(cv::Mat img) {
    if(classifier!=NULL)
    {
        //    if(util.OpenFile(configFilePath,"r")!=INI_RES::INI_SUCCESS)
        //    {
        //        std::cout<<"openConfigFileError"<<std::endl;
        //        return string("error");
        //    }
        std::cout << "---------- Prediction for "
                  << "inputImage" << " ----------" << std::endl;
        if(!checkImageFormat(img))
        {
            std::cout<<"inputImg "<<"inputImage"<<" wrong format"<<std::endl;
            //        util.CloseFile();
            return string("error");
        }
        //    string model_file   = string(util.GetStr("CarModel","modelFilePath"));
        //    string trained_file = string(util.GetStr("CarModel","trainedFilePath"));
        //    string mean_file    = string(util.GetStr("CarModel","meanFilePath"));
        //    string label_file   = string(util.GetStr("CarModel","labelFilePath"));;
        //    static Classifier classifier(model_file, trained_file, mean_file, label_file);

        std::vector<Prediction> predictions = classifier->Classify(img);
        //    util.CloseFile();
        return predictions[0].first.substr(2);
    }
    else
    {
        std::cout<<"please reInit classifier first"<<std::endl;
        return "";
    }
}

string CarModelDict::singleImagePathCarModelDict(const char* imageFilePath) {
    if(classifier!=NULL)
    {
        //    if(util.OpenFile(configFilePath,"r")!=INI_RES::INI_SUCCESS)
        //    {
        //        std::cout<<"openConfigFileError"<<std::endl;
        //        return string("error");
        //    }
        cv::Mat img=cv::imread(imageFilePath);
        std::cout << "---------- Prediction for "
                  << string(imageFilePath) << " ----------" << std::endl;
        if(!checkImageFormat(img))
        {
            std::cout<<"inputImg "<<string(imageFilePath)<<" wrong format"<<std::endl;
            //        util.CloseFile();
            return string("error");
        }
        //    string model_file   = string(util.GetStr("CarModel","modelFilePath"));
        //    string trained_file = string(util.GetStr("CarModel","trainedFilePath"));
        //    string mean_file    = string(util.GetStr("CarModel","meanFilePath"));
        //    string label_file   = string(util.GetStr("CarModel","labelFilePath"));;
        //    static Classifier classifier(model_file, trained_file, mean_file, label_file);


        std::vector<Prediction> predictions = classifier->Classify(img);
        //    util.CloseFile();
        return predictions[0].first.substr(2);
    }
    else
    {
        std::cout<<"please reInit classifier first"<<std::endl;
        return "";
    }
}

vector<string> CarModelDict::singleImagePathsCarModelDict(const vector<const char*> &imageFilePaths) {
    if(classifier!=NULL)
    {
        //    if(util.OpenFile(configFilePath,"r")!=INI_RES::INI_SUCCESS)
        //    {
        //        std::cout<<"openConfigFileError"<<std::endl;
        //        return vector<string>();
        //    }
        //    string model_file   = string(util.GetStr("CarModel","modelFilePath"));
        //    string trained_file = string(util.GetStr("CarModel","trainedFilePath"));
        //    string mean_file    = string(util.GetStr("CarModel","meanFilePath"));
        //    string label_file   = string(util.GetStr("CarModel","labelFilePath"));;
        //    static Classifier classifier(model_file, trained_file, mean_file, label_file);
        vector<string> res;
        for(vector<const char*>::const_iterator it=imageFilePaths.cbegin();it!=imageFilePaths.cend();it++)
        {
            std::cout << "---------- Prediction for "
                      << string(*it) << " ----------" << std::endl;
            string imageFilePath=string(*it);
            cv::Mat img=cv::imread(imageFilePath);
            if(!checkImageFormat(img))
            {
                std::cout<<"inputImg "<<string(imageFilePath)<<" wrong format"<<std::endl;
                res.push_back("error");
                continue;
            }
            std::vector<Prediction> predictions = classifier->Classify(img);
            res.push_back(predictions[0].first.substr(2));
        }
        //    util.CloseFile();
        return res;
    }
    else
    {
        std::cout<<"please reInit classifier first"<<std::endl;
        return vector<string>();
    }
}

vector<string> CarModelDict::singleImagesCarModelDict(const vector<cv::Mat> &images) {
    if(classifier!=NULL)
    {
        //    if(util.OpenFile(configFilePath,"r")!=INI_RES::INI_SUCCESS)
        //    {
        //        std::cout<<"openConfigFileError"<<std::endl;
        //        return vector<string>();
        //    }
        //    string model_file   = string(util.GetStr("CarModel","modelFilePath"));
        //    string trained_file = string(util.GetStr("CarModel","trainedFilePath"));
        //    string mean_file    = string(util.GetStr("CarModel","meanFilePath"));
        //    string label_file   = string(util.GetStr("CarModel","labelFilePath"));;
        //    static Classifier classifier(model_file, trained_file, mean_file, label_file);
        vector<string> res;
        int num=0;
        for(vector<cv::Mat>::const_iterator it=images.cbegin();it!=images.cend();it++)
        {
            std::cout << "---------- Prediction for "
                      << "inputImage " <<num<< " ----------" << std::endl;
            cv::Mat img=*it;
            if(!checkImageFormat(img))
            {
                std::cout<<"inputImg "<<num<<" wrong format"<<std::endl;
                res.push_back("error");
                num++;
                continue;
            }
            std::vector<Prediction> predictions = classifier->Classify(img);
            res.push_back(predictions[0].first.substr(2));
            num++;
        }
        //    util.CloseFile();
        return res;
    }
    else
    {
        std::cout<<"please reInit classifier first"<<std::endl;
        return vector<string>();
    }
}


int main(int argc,char** argv)
{
    ::google::InitGoogleLogging(argv[0]);//use only once
    CarModelDict dicter("./config.ini");//read config file and init

    cv::Mat image1=cv::imread("cars/test.png");
    cv::Mat image2=cv::imread("cars/2.jpeg");
    cv::Mat image3=cv::imread("cars/3.jpeg");
    cv::Mat image4=cv::imread("cars/4.jpeg");
    cv::Mat image5=cv::imread("cars/5.jpeg");
    cv::Mat image6=cv::imread("cars/6.jpeg");
    vector<cv::Mat> vecMat;
    vecMat.push_back(image1);
    vecMat.push_back(image2);
    vecMat.push_back(image3);
    vecMat.push_back(image4);
    vecMat.push_back(image5);
    vecMat.push_back(image6);

    const char* imagePath1="cars/1.jpeg";
    const char* imagePath2="cars/2.jpeg";
    const char* imagePath3="cars/3.jpeg";
    const char* imagePath4="cars/4.jpeg";
    const char* imagePath5="cars/5.jpeg";
    const char* imagePath6="cars/6.jpeg";

    vector<const char*> vecPath;
    vecPath.push_back(imagePath1);
    vecPath.push_back(imagePath2);
    vecPath.push_back(imagePath3);
    vecPath.push_back(imagePath4);
    vecPath.push_back(imagePath5);
    vecPath.push_back(imagePath6);


    std::cout<<"****test single Mat****"<<std::endl;
    std::cout<<dicter.singleImageCarModelDict(image1)<<std::endl;
    std::cout<<"****test single path****"<<std::endl;
    std::cout<<dicter.singleImagePathCarModelDict(imagePath1)<<std::endl;
    std::cout<<"****test Mat group****"<<std::endl;
    vector<string> vec=dicter.singleImagesCarModelDict(vecMat);
    for(vector<string>::iterator it=vec.begin();it!=vec.end();it++)
    {
        std::cout<<*it<<std::endl;
    }
    std::cout<<"****test path group****"<<std::endl;
    vector<string> vec2=dicter.singleImagePathsCarModelDict(vecPath);
    for(vector<string>::iterator it=vec2.begin();it!=vec2.end();it++)
    {
        std::cout<<*it<<std::endl;
    }
}
