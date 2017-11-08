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
	Mat img = imread("C:\\Users\\Margarita\\Downloads\\17974282_1386874394692192_2009764012_n.jpg");
	cvNamedWindow("Exemple1", CV_WINDOW_AUTOSIZE);
//	cvShowImage("Exemple1", img);
	cvWaitKey(1245);
	cvReleaseImage(&img);
	cvDestroyWindow("Exemple1");
	return 0;
}