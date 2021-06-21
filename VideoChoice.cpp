#include <opencv2/videoio.hpp>
#include <iostream>

using namespace cv;
using namespace std;

void extractImageFrame(VideoCapture& cap, Mat& cameraImage, int framesToSkip);

VideoCapture VideoChoice()
{
	VideoCapture cap;
	Mat cameraImage;
	int choice;
	cout << "Choose video:\n";
	cout << "1. Camera\n";
	cout << "2. Fire - night 1\n";
	cout << "3. Fire - day 1\n";
	cout << "Your choice: ";
	cin >> choice;
	cout << "\n";

	switch (choice)
	{
	case 1:
		cap.open(0);
		break;
	case 2:
		cap.open("C:\\Users\\Zosia\\Desktop\\FireNight1.mp4");
		break;
	case 3:
		cap.open("C:\\Users\\Zosia\\Desktop\\FireDay1.mp4");
		extractImageFrame(cap, cameraImage, 2000);
		break;
	default:
		throw invalid_argument("Wrong choice!");
	}
	if (!cap.isOpened())
	{
		cout << "Cannot open video";
		throw exception("Video choice out of range.");
	}

	return cap;
}

void extractImageFrame(VideoCapture& cap, Mat& cameraImage, int framesToSkip)
{
	for (int i = 0; i < framesToSkip; i++)
	{
		cap >> cameraImage;
	}
}