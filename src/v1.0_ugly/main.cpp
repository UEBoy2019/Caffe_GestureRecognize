/*
* date:2018.9.6
* version:0.3
* work:Call opencv to open camera and get every frames,call caffe classifier to classify every frame.
*/

#include "cv2Capturer.h"
#include "caffe_head.h"

void MenuSelete(cv2Capturer & cap) {
	//�򵥲˵�GUI���в���ʹ��
	int menuNumber = 2;
	string imgPath;	//case 3 uses.
	string menuText = "-------Menu-------\n1.Capture video and save.\n2.Capture frames and classify.\n3.Classify an image with image path\n0.Quit\n------------------\n";
	cout << menuText;
	while (cin >> menuNumber) {
		switch (menuNumber)
		{
		case 1:
			cap.captureVideo(true, true);
			break;
		case 2:
			cap.classifyCapture();
			cap.closeWindows();
			break;
		case 3:
			cout << "image path:";
			
			cin >> imgPath;
			cout << "Got path:" << imgPath << endl;
			cap.classifier->classify(imgPath);
			break;
		case 0:
			break;
		default:
			cout << "Warning: input number [" << menuNumber << "] is invaild!" << endl;
			break;
		}
		if (!menuNumber) break;
		else cout << menuText;
	}
	cap.release();
	cout << "Quited" << endl;
}

int main(int argc, char** argv) {

	//GLOG������ʾ���
	cout << "Show all caffe compiling message?(y or n):";
	if (cin.get() != 'y') 	GlobalInit(&argc, &argv);


	//��������ͷ������caffe��������Ա
	//��int������ʾ�ֶ����ò�����
	cv2Capturer cap = cv2Capturer(0);	

	//ѭ����ʾ���ܲ˵�
	MenuSelete(cap);

	system("pause");
	return 0;
}
