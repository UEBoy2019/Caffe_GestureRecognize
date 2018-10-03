#include "GestureClassifier2.h"



void GestureClassifier2::init()
{
	if (model_file.empty()) {
		LOG(WARNING)<< " Should call initFilePath first or set file path first";
		return;
	}

	//// �������
	Blob<float>* input_layer;

	Caffe::set_mode(Caffe::CPU); // ʹ��

	net_.reset(new Net<float>(model_file, TEST));	// ���������ļ����趨ģʽΪ���࣬ʵ����net����
	net_->CopyTrainedLayersFrom(trained_file);		// ����ѵ���õ�ģ���޸�ģ�Ͳ���

													//// �������Ϣ
	input_layer = net_->input_blobs()[0];
	num_channels_ = input_layer->channels();
	LOG(INFO) << "classifier: num_channels_: " << num_channels_;
	input_geometry_ = cv::Size(input_layer->width(), input_layer->height());	//cv::mat shape_[3] and  shape_[2].

	//// �����ֵ�ļ����õ���ֵͼ��
	BlobProto blob_proto;
	ReadProtoFromBinaryFileOrDie(mean_file.c_str(), &blob_proto);
	Blob<float> mean_blob;
	mean_blob.FromProto(blob_proto);
	vector<cv::Mat> channels;
	float* data = mean_blob.mutable_cpu_data();
	for (int i = 0; i < num_channels_; i++)
	{
		cv::Mat channel(mean_blob.height(), mean_blob.width(), CV_32FC1, data);
		channels.push_back(channel);
		data += mean_blob.height() * mean_blob.width();
	}

	cv::Mat mean;
	merge(channels, mean);
	cv::Scalar channel_mean = cv::mean(mean);
	mean_ = cv::Mat(input_geometry_, mean.type(), channel_mean);

	//// ��ȡ��ǩ
	std::ifstream labels(label_file.c_str());
	string line;
	while (getline(labels, line))
		labels_.push_back(string(line));
	//�жϱ�ǩ��������ģ������������Ƿ���ͬ
	Blob<float>* output_layer = net_->output_blobs()[0];
	LOG(INFO) << "output_layer dimension: " << output_layer->channels()
		<< "; labels num ber: " << labels_.size();

	// Ԥ��ͼ����Ϣ
	input_layer->Reshape(1, num_channels_, input_geometry_.height, input_geometry_.width);
	net_->Reshape();

	//��input_channelsָ��ģ�͵���������λ��
	int width = input_layer->width();
	int height = input_layer->height();
	float* input_data = input_layer->mutable_cpu_data();
	for (int i = 0; i < input_layer->channels(); i++)
	{
		cv::Mat channel(height, width, CV_32FC1, input_data);
		input_channels.push_back(channel);
		input_data += width * height;
	}
}

cv::Mat GestureClassifier2::getIntputMat(const string img_file_path)
{
	//// �ı�ͼ��Ĵ�С��ͨ�����������ͣ�ȥ��ֵ��
	cv::Mat img = cv::imread(img_file_path);
	cv::Mat sample;
	if (img.channels() == 3 && num_channels_ == 1)		cv::cvtColor(img, sample, cv::COLOR_BGR2GRAY);
	else if (img.channels() == 4 && num_channels_ == 1)	cv::cvtColor(img, sample, cv::COLOR_BGRA2GRAY);
	else if (img.channels() == 4 && num_channels_ == 3)	cv::cvtColor(img, sample, cv::COLOR_BGRA2BGR);
	else if (img.channels() == 1 && num_channels_ == 3)	cv::cvtColor(img, sample, cv::COLOR_GRAY2BGR);
	else
		sample = img;

	cv::Mat sample_resized;
	if (sample.size() != input_geometry_)	cv::resize(sample, sample_resized, input_geometry_);
	else									sample_resized = sample;

	cv::Mat sample_float;
	if (num_channels_ == 3)		sample_resized.convertTo(sample_float, CV_32FC3);
	else						sample_resized.convertTo(sample_float, CV_32FC1);

	cv::Mat sample_normalized;
	cv::subtract(sample_float, mean_, sample_normalized);

	return sample_normalized;
}

cv::Mat GestureClassifier2::getIntputMat(cv::Mat & img)
{
	cv::Mat sample;
	if (img.channels() == 3 && num_channels_ == 1)		cv::cvtColor(img, sample, cv::COLOR_BGR2GRAY);
	else if (img.channels() == 4 && num_channels_ == 1)	cv::cvtColor(img, sample, cv::COLOR_BGRA2GRAY);
	else if (img.channels() == 4 && num_channels_ == 3)	cv::cvtColor(img, sample, cv::COLOR_BGRA2BGR);
	else if (img.channels() == 1 && num_channels_ == 3)	cv::cvtColor(img, sample, cv::COLOR_GRAY2BGR);
	else
		sample = img;

	cv::Mat sample_resized;
	if (sample.size() != input_geometry_)	cv::resize(sample, sample_resized, input_geometry_);
	else									sample_resized = sample;

	cv::Mat sample_float;
	if (num_channels_ == 3)		sample_resized.convertTo(sample_float, CV_32FC3);
	else						sample_resized.convertTo(sample_float, CV_32FC1);

	cv::Mat sample_normalized;
	cv::subtract(sample_float, mean_, sample_normalized);

	return sample_normalized;
	
}

void GestureClassifier2::classify(string img_file)
{
	//// ����õ����ݱ���������㣨ָ��ָ��ʵ�֣�
	cv::split(getIntputMat(img_file), input_channels);

	//// ����ģ�ͽ���Ԥ��
	net_->Forward();

	//// ��������
	Blob<float>* output_layer = net_->output_blobs()[0];
	const float* begin = output_layer->cpu_data();
	const float* end = begin + output_layer->channels();
	vector<float> output = vector<float>(begin, end);

	//// ��ʾ����ǰN��Ľ��
	int N = 10;
	N = std::min<int>(labels_.size(), N);
	std::vector<int> maxN = Argmax(output, N);


	std::vector<Prediction> predictions;
	for (int i = 0; i < N; ++i) {
		int idx = maxN[i];
		predictions.push_back(std::make_pair(labels_[idx], output[idx]));
	} 

	for (size_t i = 0; i < predictions.size(); ++i) {
		Prediction p = predictions[i];
		std::cout << std::fixed << std::setprecision(4) << p.second << " - \""
			<< p.first << "\"" << std::endl;
	}
}

Prediction GestureClassifier2::getBestPrediction(cv::Mat & img)
{
	//// ����õ����ݱ���������㣨ָ��ָ��ʵ�֣�
	cv::split(getIntputMat(img), input_channels);

	//// ����ģ�ͽ���Ԥ��
	net_->Forward();

	//// ��������
	Blob<float>* output_layer = net_->output_blobs()[0];
	const float* begin = output_layer->cpu_data();
	const float* end = begin + output_layer->channels();
	vector<float> output = vector<float>(begin, end);

	//// ��ʾ����ǰN��Ľ��	���˴��ɸĽ�Ϊ��һ����
	int N = 1;
	N = std::min<int>(labels_.size(), N);
	std::vector<int> maxN = Argmax(output, N);

	Prediction bestP = std::make_pair(labels_[maxN[0]], output[maxN[0]]);

	return bestP;
}


