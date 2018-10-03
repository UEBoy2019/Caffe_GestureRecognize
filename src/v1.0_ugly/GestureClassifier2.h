#pragma once
#include <caffe/caffe.hpp>
#include <string>
#include <opencv2/opencv.hpp>
#define IF_TEST_MODE(x) x;

using namespace caffe;
using namespace std;

typedef pair<string, float> Prediction;	//��ǩԤ������һ�Լ�ֵ��

////static ���� Ϊ��̬��ȫ�ֺ�����������Դ���ļ�ʹ�á�
// ����Argmax()��Ҫ�õ����Ӻ�����vector������
static bool PairCompare(const std::pair<float, int>& lhs,
	const std::pair<float, int>& rhs) {
	return lhs.first > rhs.first;
}
// ����Ԥ�����и��ʴӴ�С��ǰN��Ԥ����������
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

//@caffeԤ�⹤����
class GestureClassifier2
{
	string model_file;
	string trained_file;
	string mean_file;
	string label_file;

	////�ɸ��ò�����
	boost::shared_ptr<Net<float> > net_;	//DtypeΪfloat��Net�࣬��������Ȩ����ָ��shared_ptrָ��������
											//�����ÿ�����ô���+1���Ͽ�����ʱ-1�����ô���Ϊ0ʱ�Զ���������
											//.����ָ�����Ա��->����ָ����ָ��������Ա��
	cv::Size input_geometry_;
	int num_channels_;
	cv::Mat mean_;
	vector<string> labels_;
	vector<cv::Mat> input_channels;



public:
	inline long getNetPtrUseCount() {
		IF_TEST_MODE(cout << "Message: classifier: \tshared_ptr net_ use count:" << net_.use_count() << endl)
		return net_.use_count();
	}

	inline void initFilePath(string model = R"(.\data\hand_net.prototxt)",
		string trained = R"(.\data\1miohands-v2.caffemodel)",
		string mean = R"(.\data\227x227-TRAIN-allImages-forFeatures-0label-227x227handpatch.mean)",
		string label = R"(.\data\s1.txt)") {
		//// ����ģ�������ļ���ģ���ļ�����ֵ�ļ�����ǩ�ļ�·��
		model_file = model;
		trained_file = trained;
		mean_file = mean;
		label_file = label;
		
	}

	void init();
	cv::Mat getIntputMat(const string);
	cv::Mat getIntputMat(cv::Mat&);	//ֱ�����ô�����ͷ���֡��ɫ�ʾ���
	void classify(string);
	Prediction getBestPrediction(cv::Mat&);


	GestureClassifier2() {		//�Զ���ʼ��
		initFilePath();
		init();
	};		
	GestureClassifier2(int i) {	//���ֶ�init
		LOG(WARNING) << "GestureClassifier2 need manual init";
	};	
	~GestureClassifier2() {};
};

