#ifndef CAR_DETECT_H_
#define CAR_DETECT_H_

#include <caffe/caffe.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <algorithm>
#include <iomanip>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>
#include <vector>
               
using namespace cv;


using namespace caffe;

class Detector {
public:
	Detector(const string& model_file,
		const string& weights_file,
		const string& mean_file,
		const string& mean_value);

	std::vector<vector<float> > Detect(const cv::Mat& img);

private:
	void SetMean(const string& mean_file, const string& mean_value);

	void WrapInputLayer(std::vector<cv::Mat>* input_channels);

	void Preprocess(const cv::Mat& img,
		std::vector<cv::Mat>* input_channels);

private:
    shared_ptr<Net<float> > net_;
	cv::Size input_geometry_;
	int num_channels_;
	cv::Mat mean_;
};

Detector::Detector(const string& model_file,
	const string& weights_file,
	const string& mean_file,
	const string& mean_value) {
#ifdef CPU_ONLY
	Caffe::set_mode(Caffe::CPU);
#else
	Caffe::set_mode(Caffe::GPU);
#endif
    net_.reset(new Net<float>(model_file, TEST));
	net_->CopyTrainedLayersFrom(weights_file);

	CHECK_EQ(net_->num_inputs(), 1) << "Network should have exactly one input.";
	CHECK_EQ(net_->num_outputs(), 1) << "Network should have exactly one output.";

	Blob<float>* input_layer = net_->input_blobs()[0];
	num_channels_ = input_layer->channels();
	CHECK(num_channels_ == 3 || num_channels_ == 1)
		<< "Input layer should have 1 or 3 channels.";
	input_geometry_ = cv::Size(input_layer->width(), input_layer->height());

	SetMean(mean_file, mean_value);
}

std::vector<vector<float> > Detector::Detect(const cv::Mat& img) {
	Blob<float>* input_layer = net_->input_blobs()[0];
	input_layer->Reshape(1, num_channels_, input_geometry_.height, input_geometry_.width);
	net_->Reshape();
	std::vector<cv::Mat> input_channels;
	WrapInputLayer(&input_channels);
	Preprocess(img, &input_channels);
	net_->Forward();
	Blob<float>* result_blob = net_->output_blobs()[0];
	const float* result = result_blob->cpu_data();
	const int num_det = result_blob->height();
	vector<vector<float> > detections;
	for (int k = 0; k < num_det; ++k) {
		if (result[0] == -1) {
			result += 7;
			continue;
		}
		vector<float> detection(result, result + 7);
		detections.push_back(detection);
		result += 7;
	}
	vector<vector<float> > results;
	for (int i = 0; i < detections.size(); ++i) {
		const vector<float>& d = detections[i];
		// 返回格式: [ID, 类别, 置信度, xmin, ymin, xmax, ymax].
		if (d[1] == 7){
			vector<float> result;
			result.push_back(d[2]);
			if (d[3] < 0)
				result.push_back(0.0 * img.cols);
			else
				result.push_back(d[3] * img.cols);
			if (d[4] < 0)
				result.push_back(0.0 * img.rows);
			else
				result.push_back(d[4] * img.rows);
			if (d[5] > 1)
				result.push_back(1.0 * img.cols);
			else
				result.push_back(d[5] * img.cols);
			if (d[6] > 1)
				result.push_back(1.0 * img.rows);
			else
				result.push_back(d[6] * img.rows);
			results.push_back(result);
		}
		
	}
	return results;
}

void Detector::SetMean(const string& mean_file, const string& mean_value) {
	cv::Scalar channel_mean;
	if (!mean_file.empty()) {
		CHECK(mean_value.empty()) <<
			"Cannot specify mean_file and mean_value at the same time";
		BlobProto blob_proto;
		ReadProtoFromBinaryFileOrDie(mean_file.c_str(), &blob_proto);

		Blob<float> mean_blob;
		mean_blob.FromProto(blob_proto);
		CHECK_EQ(mean_blob.channels(), num_channels_)
			<< "Number of channels of mean file doesn't match input layer.";

		std::vector<cv::Mat> channels;
		float* data = mean_blob.mutable_cpu_data();
		for (int i = 0; i < num_channels_; ++i) {
			cv::Mat channel(mean_blob.height(), mean_blob.width(), CV_32FC1, data);
			channels.push_back(channel);
			data += mean_blob.height() * mean_blob.width();
		}

		cv::Mat mean;
		cv::merge(channels, mean);

		channel_mean = cv::mean(mean);
		mean_ = cv::Mat(input_geometry_, mean.type(), channel_mean);
	}
	if (!mean_value.empty()) {
		CHECK(mean_file.empty()) <<
			"Cannot specify mean_file and mean_value at the same time";
		stringstream ss(mean_value);
		vector<float> values;
		string item;
		while (getline(ss, item, ',')) {
			float value = std::atof(item.c_str());
			values.push_back(value);
		}
		CHECK(values.size() == 1 || values.size() == num_channels_) <<
			"Specify either 1 mean_value or as many as channels: " << num_channels_;

		std::vector<cv::Mat> channels;
		for (int i = 0; i < num_channels_; ++i) {
			cv::Mat channel(input_geometry_.height, input_geometry_.width, CV_32FC1,
				cv::Scalar(values[i]));
			channels.push_back(channel);
		}
		cv::merge(channels, mean_);
	}
}

void Detector::WrapInputLayer(std::vector<cv::Mat>* input_channels) {
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

void Detector::Preprocess(const cv::Mat& img,
	std::vector<cv::Mat>* input_channels) {
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
	cv::split(sample_normalized, *input_channels);

	CHECK(reinterpret_cast<float*>(input_channels->at(0).data)
		== net_->input_blobs()[0]->cpu_data())
		<< "Input channels are not wrapping the input layer of the network.";
}

DEFINE_string(mean_file, "",
	"The mean file used to subtract from the input image.");
DEFINE_string(mean_value, "104,117,123",
	"If specified, can be one value or can be same as image channels"
	" - would subtract from the corresponding channel). Separated by ','."
	"Either mean_file or mean_value should be provided, not both.");
DEFINE_string(file_type, "image",
	"The file type in the list_file. Currently support image and video.");
DEFINE_string(out_file, "",
	"If provided, store the detection results in the out_file.");
DEFINE_double(confidence_threshold, 0.1,
	"Only store detections with score higher than the threshold.");

#endif  // CAR_DETECT_H_
