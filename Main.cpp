#include <opencv2/videoio.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/highgui.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <cstdlib>
#include <conio.h>
#include <Windows.h>

#include "Header.h"

using namespace cv;
using namespace std;

auto minRedValue = 190;
auto minGreenValue = 100;
auto maxBlueValue = 140;
auto blurImage = 0;
auto MOG2_pointer = createBackgroundSubtractorMOG2();

auto history = 50;
auto nMixtures = 8;
auto shadows = 0;

const Point communique = Point(50, 50);
auto fireDetectionSize = 50;
const auto morphSize = 1;
const auto morphElement = 1;
const auto kernel = getStructuringElement(morphElement, Size(2 * morphSize + 1, 2 * morphSize + 1),
	Point(morphSize, morphSize));

void extractRGBandYCbCr(Mat& inputVideo, Mat& extractRGBandYCbCr);
void detectFire(Mat& inputVideo, Mat& YCbCrVideo, Mat& firePix);
void fireDetectionAlarm(Mat& cameraImage, double& contoursSize);
void createTrackbars(int& history, int& nMixtures, int& shadows, int& blurImage, int& fireDetectionSize,int& minRedValue, int& minGreenValue, int& maxBlueValue);
void updateTrackbar(int, void*);
double drawContoursAndGetSize(Mat& cameraImage, Mat& segmentedFire, vector<vector<Point>>& contours);

int main() {
	auto cap = VideoChoice();
	createTrackbars(history, nMixtures, shadows, blurImage, fireDetectionSize, minRedValue, minGreenValue, maxBlueValue);

	Mat inputVideo;
	Mat YCbCrVideo;
	Mat firePix;
	Mat foreground;
	Mat background;
	Mat segmentedFire;
	vector<vector<Point>> contours;

	while (true) {
		cap >> inputVideo;
		if (inputVideo.empty()) {
			break;
		}

		try {

			if (blurImage == 1)
			{
				morphologyEx(inputVideo, inputVideo, MORPH_OPEN, kernel);
				morphologyEx(inputVideo, inputVideo, MORPH_DILATE, kernel);
			}

			//Extract R,G,B and Y,Cb, Cr component
			extractRGBandYCbCr(inputVideo, YCbCrVideo);

			//fire pixels
			detectFire(inputVideo, YCbCrVideo, firePix);

			MOG2_pointer->apply(firePix, foreground);
			MOG2_pointer->getBackgroundImage(background);

			findContours(foreground, contours, RETR_LIST, CHAIN_APPROX_NONE);
			auto contoursSize = drawContoursAndGetSize(inputVideo, segmentedFire, contours);

			fireDetectionAlarm(inputVideo, contoursSize);

			imshow("Image", inputVideo);
			//imshow("Ycbcr", YCbCrVideo);
			//imshow("Fire", firePix);
			//imshow("Segmented fire", segmentedFire);
		}
		catch(Exception& e){
			cout << "Error: " << e.msg << endl;
		}

		int k = waitKey(30);
		if (k != -1){
			break;
		}
	}

	MOG2_pointer.release();
	cap.release();
	destroyAllWindows();
	return 0;
}



void extractRGBandYCbCr(Mat& inputVideo, Mat& extractRGBandYCbCr)
{
	extractRGBandYCbCr = Mat(inputVideo.rows, inputVideo.cols, CV_32FC3);
	for (int i = 0; i < inputVideo.rows; i++){
		for (int j = 0; j < inputVideo.cols; j++){
			Vec3b bgrPixel = inputVideo.at<Vec3b>(i, j);
			float B = bgrPixel[0];
			float G = bgrPixel[1];
			float R = bgrPixel[2];

			float delta = 0.5f;
			float Y = 0.299f * R + 0.587f * G + 0.114f * B;
			float Cb = (B - Y) * 0.564f + delta;
			float Cr = (R - Y) * 0.713f + delta;

			Vec3f ycbcrPixel(Y, Cb, Cr);
			extractRGBandYCbCr.at<Vec3f>(i, j) = ycbcrPixel;
		}
	}
}

void detectFire(Mat& inputVideo, Mat& YCbCrVideo, Mat& firePix) {
	firePix = Mat(inputVideo.rows, inputVideo.cols, inputVideo.type());
	for (auto i = 0; i < inputVideo.rows; i++){
		for (auto j = 0; j < inputVideo.cols; j++){
			Vec3b rgbPixel = inputVideo.at<Vec3b>(i, j);
			auto B = rgbPixel[0];
			auto G = rgbPixel[1];
			auto R = rgbPixel[2];
	
			Vec3f ycbcrPixel = YCbCrVideo.at<Vec3f>(i, j);
	
			auto Y = ycbcrPixel[0];
			auto Cb = ycbcrPixel[1];
			auto Cr = ycbcrPixel[2];
			if((R > G && G > B) && (R > minRedValue && G > minGreenValue && B < maxBlueValue) && (Y >= Cb) && (Cr >= Cb)){
				firePix.at<Vec3b>(i, j) = Vec3b(B, G, R);
			}else{
				firePix.at<Vec3b>(i, j) = Vec3b(0, 0, 0);
			}
		}
	}
}

void fireDetectionAlarm(Mat& inputVideo, double& contoursSize)
{
	if (contoursSize > fireDetectionSize){
		putText(inputVideo, "Fire detected!", communique, FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255));
	}else{
		putText(inputVideo, "No fire", communique, FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 128, 0));
	}
}

double drawContoursAndGetSize(Mat& inputVideo, Mat& segmentedFire, vector<vector<Point>>& contours)
{
	segmentedFire = Mat::zeros(inputVideo.rows, inputVideo.cols, CV_8UC1);

	double contoursSize = 0;
	for (auto i = 0; i < contours.size(); i++){
		contoursSize += contourArea(contours[i]);
		drawContours(segmentedFire, contours, i, 255, 1);
	}
	return contoursSize;
}

void createTrackbars(int& history, int& nMixtures, int& shadows, int& blurImage, int& fireDetectionSize,
	int& minRedValue, int& minGreenValue, int& maxBlueValue)
{
	namedWindow("Settings");
	const auto settingsWidth = 640;
	const auto settingsHeight = 480;
	resizeWindow("Settings", settingsWidth, settingsHeight);
	createTrackbar("History", "Settings", &history, 100, updateTrackbar);
	createTrackbar("nMixtures", "Settings", &nMixtures, 50, updateTrackbar);
	createTrackbar("Shadows", "Settings", &shadows, 1, updateTrackbar);
	createTrackbar("Blur", "Settings", &blurImage, 1);
	createTrackbar("Fire Size", "Settings", &fireDetectionSize, 1000);
	createTrackbar("Min Red", "Settings", &minRedValue, 255);
	createTrackbar("Min Green", "Settings", &minGreenValue, 255);
	createTrackbar("Max Blue", "Settings", &maxBlueValue, 255);
}

void updateTrackbar(int, void*)
{
	MOG2_pointer.release();
	MOG2_pointer = createBackgroundSubtractorMOG2();
	MOG2_pointer->setHistory(history);
	if (nMixtures == 0)
	{
		nMixtures += 1;
	}
	MOG2_pointer->setNMixtures(nMixtures);
	MOG2_pointer->setDetectShadows(shadows);
}