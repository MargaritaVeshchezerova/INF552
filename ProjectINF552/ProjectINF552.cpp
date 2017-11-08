// ProjectINF552.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>

using namespace cv;
using namespace std;


int main()
{
	IplImage* img = cvLoadImage("C:\\Users\\Margarita\\Downloads\\17974282_1386874394692192_2009764012_n.jpg");
	cvNamedWindow("Blabla", CV_WINDOW_AUTOSIZE);
//	cvShowImage("Blabla", img);
	cvWaitKey(0);
	cvReleaseImage(&img);
	cvDestroyWindow("Blabla");
	return 0;
}