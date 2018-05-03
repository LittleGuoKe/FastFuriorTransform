#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>
#include <cvaux.h>
#include <Windows.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define PI 3.14159

using namespace cv;
using namespace std;

struct eType {
	float real;
	float irreal;
};

eType eMult(eType a, eType b) {
	return{ a.real*b.real - a.irreal*b.irreal, a.real*b.irreal + a.irreal*b.real };
}

eType eAdd(eType a, eType b) {
	return{ a.real + b.real, a.irreal + b.irreal };
}

int reverse_bit(int num, int len) //�������������
{
	int i, bit;
	unsigned new_num = 0;
	for (i = 0; i < len; i++)
	{
		bit = num & 1;
		new_num <<= 1;
		new_num = new_num | bit;
		num >>= 1;
	}
	return new_num;
}

int if_binaryNum(int length) { //�ж��Ƿ���2�������η�
	int num = 0;
	while (length != 1) {
		if (length % 2 == 0) {
			length = length / 2;
			num++;
		}
		else {
			return -1;
		}
	}
	return num;
}

Mat binarylizeImage(Mat image) {
	float c = image.cols, r = image.rows;
	int cn = 0, rn = 0, cnew = 2, rnew = 2;
	while (c / 2 > 1) { c = c / 2; cn++;}
	while (r / 2 > 1) { r = r / 2; rn++;}
	while (cn > 0) { cnew = cnew * 2; cn--;}
	while (rn > 0) { rnew = rnew * 2; rn--;}
	resize(image, image, Size(cnew, rnew));
	return image;
}

void fastFuriorTransform(Mat image) {
	int lengthC = image.cols;
	int lengthR = image.rows;
	int numC, numR;
	vector<eType> resultE;
	Mat furiorResultF = Mat(image.cols, image.rows, CV_32FC1);
	//ӳ���
	vector <int> mappingC;
	vector <int> mappingR;
	//Wֵ��
	vector <eType> mappingWC;
	vector <eType> mappingWR;

	//�ж�����ͼƬ�߳��Ƿ���2��n�η�����������ϣ�����image��С
	numC = if_binaryNum(lengthC);
	numR = if_binaryNum(lengthR);
	if (numC == -1 || numR == -1) {
		fastFuriorTransform(binarylizeImage(image));
		return;
	}

	//����ӳ���
	for (int c = 0; c < image.cols; c++) {
		mappingC.push_back(0);
	}
	for (int r = 0; r < image.rows; r++) {
		mappingR.push_back(0);
	}
	for (int c = 0; c < image.cols; c++) {
		mappingC.at(reverse_bit(c, numC)) = c;
	}
	for (int r = 0; r < image.rows; r++) {
		mappingR.at(reverse_bit(r, numR)) = r;
	}

	//����W��
	for (int i = 0; i < lengthC / 2; i++) {
		eType w = { cosf(2 * PI / lengthC * i), -1 * sinf(2 * PI / lengthC * i) };
		mappingWC.push_back(w);
	}
	for (int i = 0; i < lengthR / 2; i++) {
		eType w = { cosf(2 * PI / lengthR * i), -1 * sinf(2 * PI / lengthR * i) };
		mappingWR.push_back(w);
	}

	//��ʼ��
	for (int r = 0; r < lengthR; r++) {
		for (int c = 0; c < lengthC; c++) {
			//����ӳ���������0��1�����32λ�������ʹ洢�Ҷ�ֵ
			eType w = { (float)image.at<uchar>(mappingR.at(r), mappingC.at(c)) / 255, 0 };
			resultE.push_back(w);
		}
	}

	//ѭ������ÿ��
	for (int r = 0; r < lengthR; r++) {
		//ѭ������resultE�е�ǰ�е���ֵ�������յ�����ǰ����ƽ�
		for (int i = 0; i < numC; i++) {
			int combineSize = 2 << i;
			vector<eType> newRow;
			//����2,4,8,16...Ϊ��λ���кϲ��������½ڵ��ֵ
			for (int j = 0; j < lengthC; j = j + combineSize) {
				int n;
				for (int k = 0; k < combineSize; k++) {
					if (k < (combineSize >> 1)) {
						int w = k * lengthC / combineSize;
						n = k + j + r*lengthC;
						newRow.push_back(eAdd(resultE.at(n), eMult(resultE.at(n + (combineSize >> 1)), mappingWC.at(w))));
					}
					else {
						int w = (k - (combineSize >> 1)) * lengthC / combineSize;
						n = k + j - (combineSize >> 1) + r*lengthC;
						newRow.push_back(eAdd(resultE.at(n), eMult({ -1, 0 }, eMult(resultE.at(n + (combineSize >> 1)), mappingWC.at(w)))));
					}

				}
			}
			//��newRow������resultE�е�ֵ
			for (int j = 0; j < lengthC; j++) {
				int n = j + r*lengthC;
				resultE.at(n) = newRow.at(j);
			}
			newRow.clear();
		}
	}

	//ѭ������ÿ��
	for (int c = 0; c < lengthC; c++) {
		for (int i = 0; i < numR; i++) {
			int combineSize = 2 << i;
			vector <eType> newColum;
			for (int j = 0; j < lengthR; j = j + combineSize) {
				int n;
				for (int k = 0; k < combineSize; k++) {
					if (k < (combineSize >> 1)) {
						int w = k * lengthR / combineSize;
						n = (j + k) * lengthC + c;
						newColum.push_back(eAdd(resultE.at(n), eMult(resultE.at(n + (combineSize >> 1)*lengthC), mappingWR.at(w))));
					}
					else {
						int w = (k - (combineSize >> 1)) * lengthR / combineSize;
						n = (j + k - (combineSize >> 1)) * lengthC + c;
						newColum.push_back(eAdd(resultE.at(n), eMult({ -1, 0 }, eMult(resultE.at(n + (combineSize >> 1)*lengthC), mappingWR.at(w)))));
					}
				}
			}
			//��newColum������resultE�е�ֵ
			for (int j = 0; j < lengthR; j++) {
				int n = j*lengthC + c;
				resultE.at(n) = newColum.at(j);
			}
			newColum.clear();
		}
	}

	//�������һ��vector<float>��
	float val_max, val_min;
	vector <float> amplitude;
	for (int r = 0; r < lengthR; r++) {
		for (int c = 0; c < lengthC; c++) {
			eType e = resultE.at(r*lengthC + c);
			float val = sqrt(e.real*e.real + e.irreal*e.irreal) + 1;
			//�����߶�����
			val = log(val);
			amplitude.push_back(val);
			if (c == 0 && r == 0) {
				val_max = val;
				val_min = val;
			}
			else {
				if (val_max < val) val_max = val;
				if (val_min > val) val_min = val;
			}
		}
	}

	//��vector�е�����ת�浽Mat�У�����һ����0��255����
	Mat fftResult = Mat(lengthC, lengthR, CV_8UC1);
	for (int i = 0; i < lengthR; i++) {
		for (int j = 0; j < lengthC; j++) {
			int val = (int)((amplitude.at(i*lengthC + j) - val_min) * 255 / (val_max - val_min));
			fftResult.at<uchar>(i, j) = val;
		}
	}

	//��������
	int cx = fftResult.cols / 2;
	int cy = fftResult.rows / 2;
	Mat q0(fftResult, Rect(0, 0, cx, cy));
	Mat q1(fftResult, Rect(cx, 0, cx, cy));
	Mat q2(fftResult, Rect(0, cy, cx, cy));
	Mat q3(fftResult, Rect(cx, cy, cx, cy));

	Mat tmp;
	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);
	q1.copyTo(tmp);
	q2.copyTo(q1);
	tmp.copyTo(q2);
	
	imwrite("fft.jpg", fftResult);
	imshow("fft", fftResult);
}

int main(void) {
	char imagePath[256];
	time_t start, end;
	printf("Please input the path of the image :(No more than 255 words)\n");
	//Attention, this place may have some overflow problems.
	cin.getline(imagePath, 256);
	Mat inputImage = imread(imagePath, 0);

	while (!inputImage.data) {
		printf("<Error-404>: Can't find this file!\n");
		printf("Please input the path of the image:\n");
		cin.getline(imagePath, 256);
		inputImage = imread(imagePath, 0);
	}

	time(&start);
	fastFuriorTransform(inputImage);
	time(&end);
	printf("Total Cost: %fs\n", difftime(end, start));

	waitKey();

	return 0;
}

