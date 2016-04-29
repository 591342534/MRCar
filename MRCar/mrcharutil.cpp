#include "mrcharutil.h"

float countOfBigValue(Mat &mat, int iValue) {
	float iCount = 0.0;
	if (mat.rows > 1) {
		for (int i = 0; i < mat.rows; ++i) {
			if (mat.data[i * mat.step[0]] > iValue) {
				iCount += 1.0;
			}
		}
		return iCount;

	}
	else {
		for (int i = 0; i < mat.cols; ++i) {
			if (mat.data[i] > iValue) {
				iCount += 1.0;
			}
		}

		return iCount;
	}
}

Mat ProjectedHistogram(Mat img, int t) {
	int sz = (t) ? img.rows : img.cols;
	Mat mhist = Mat::zeros(1, sz, CV_32F);

	for (int j = 0; j < sz; j++) {
		Mat data = (t) ? img.row(j) : img.col(j);

		//ͳ����һ�л�һ���У�����Ԫ�صĸ����������浽mhist��

		mhist.at<float>(j) = countOfBigValue(data, 20);
	}

	// Normalize histogram
	double min, max;
	minMaxLoc(mhist, &min, &max);

	//��mhistֱ��ͼ�е����ֵ����һ��ֱ��ͼ

	if (max > 0)
		mhist.convertTo(mhist, -1, 1.0f / max, 0);

	return mhist;
}

Mat getTheFeatures(Mat in) {
	const int VERTICAL = 0;
	const int HORIZONTAL = 1;

	// Histogram features
	Mat vhist = ProjectedHistogram(in, VERTICAL);
	Mat hhist = ProjectedHistogram(in, HORIZONTAL);

	// Last 10 is the number of moments components
	int numCols = vhist.cols + hhist.cols;

	Mat out = Mat::zeros(1, numCols, CV_32F);

	// Asign values to feature,��������Ϊˮƽ����ֱֱ��ͼ

	int j = 0;
	for (int i = 0; i < vhist.cols; i++) {
		out.at<float>(j) = vhist.at<float>(i);
		j++;
	}
	for (int i = 0; i < hhist.cols; i++) {
		out.at<float>(j) = hhist.at<float>(i);
		j++;
	}

	return out;
}

//! EasyPR��getFeatures�ص�����
//! �������ǻ�ȡ��ֱ��ˮƽ��ֱ��ͼͼֵ

void getHistogramFeatures(const Mat& image, Mat& features) {
	Mat grayImage;
	cvtColor(image, grayImage, CV_RGB2GRAY);

	//grayImage = histeq(grayImage);

	Mat img_threshold;
	threshold(grayImage, img_threshold, 0, 255,
		CV_THRESH_OTSU + CV_THRESH_BINARY);
	features = getTheFeatures(img_threshold);
}
#define HORIZONTAL 1
#define VERTICAL 0

Mat CutTheRect(Mat &in, Rect &rect) {
	int size = in.cols;  // (rect.width>rect.height)?rect.width:rect.height;
	Mat dstMat(size, size, CV_8UC1);
	dstMat.setTo(Scalar(0, 0, 0));

	int x = (int)floor((float)(size - rect.width) / 2.0f);
	int y = (int)floor((float)(size - rect.height) / 2.0f);

	//��rect�е����� ��ȡ��dstMat���м�

	for (int i = 0; i < rect.height; ++i) {

		//��

		for (int j = 0; j < rect.width; ++j) {
			dstMat.data[dstMat.step[0] * (i + y) + j + x] =
				in.data[in.step[0] * (i + rect.y) + j + rect.x];
		}
	}

	//
	return dstMat;
}

Rect GetCenterRect(Mat &in) {
	Rect _rect;

	int top = 0;
	int bottom = in.rows - 1;

	//����

	for (int i = 0; i < in.rows; ++i) {
		bool bFind = false;
		for (int j = 0; j < in.cols; ++j) {
			if (in.data[i * in.step[0] + j] > 20) {
				top = i;
				bFind = true;
				break;
			}
		}
		if (bFind) {
			break;
		}

		//ͳ����һ�л�һ���У�����Ԫ�صĸ���

	}
	for (int i = in.rows - 1;
		i >= 0;
		--i) {
		bool bFind = false;
		for (int j = 0; j < in.cols; ++j) {
			if (in.data[i * in.step[0] + j] > 20) {
				bottom = i;
				bFind = true;
				break;
			}
		}
		if (bFind) {
			break;
		}

		//ͳ����һ�л�һ���У�����Ԫ�صĸ���

	}

	//����

	int left = 0;
	int right = in.cols - 1;
	for (int j = 0; j < in.cols; ++j) {
		bool bFind = false;
		for (int i = 0; i < in.rows; ++i) {
			if (in.data[i * in.step[0] + j] > 20) {
				left = j;
				bFind = true;
				break;
			}
		}
		if (bFind) {
			break;
		}

		//ͳ����һ�л�һ���У�����Ԫ�صĸ���

	}
	for (int j = in.cols - 1;
		j >= 0;
		--j) {
		bool bFind = false;
		for (int i = 0; i < in.rows; ++i) {
			if (in.data[i * in.step[0] + j] > 20) {
				right = j;
				bFind = true;

				break;
			}
		}
		if (bFind) {
			break;
		}

		//ͳ����һ�л�һ���У�����Ԫ�صĸ���

	}

	_rect.x = left;
	_rect.y = top;
	_rect.width = right - left + 1;
	_rect.height = bottom - top + 1;

	return _rect;
}

Mat features(Mat in, int sizeData) {

	//��ȡ�м�����

	Rect _rect = GetCenterRect(in);

	Mat tmpIn = CutTheRect(in, _rect);
	// Mat tmpIn = in.clone();
	// Low data feature
	Mat lowData;
	resize(tmpIn, lowData, Size(sizeData, sizeData));

	// Histogram features
	Mat vhist = ProjectedHistogram(lowData, VERTICAL);
	Mat hhist = ProjectedHistogram(lowData, HORIZONTAL);

	// Last 10 is the number of moments components
	int numCols = vhist.cols + hhist.cols + lowData.cols * lowData.cols;

	Mat out = Mat::zeros(1, numCols, CV_32F);
	// Asign values to

	// feature,ANN����������Ϊˮƽ����ֱֱ��ͼ�͵ͷֱ���ͼ������ɵ�ʸ��

	int j = 0;
	for (int i = 0; i < vhist.cols; i++) {
		out.at<float>(j) = vhist.at<float>(i);
		j++;
	}
	for (int i = 0; i < hhist.cols; i++) {
		out.at<float>(j) = hhist.at<float>(i);
		j++;
	}
	for (int x = 0; x < lowData.cols; x++) {
		for (int y = 0; y < lowData.rows; y++) {
			out.at<float>(j) += (float)lowData.at < unsigned
				char >(x, y);
			j++;
		}
	}
	return out;
}