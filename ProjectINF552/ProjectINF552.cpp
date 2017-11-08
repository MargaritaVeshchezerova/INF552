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
	Mat img = imread("./Images/cathedral/d001.jpg");
	namedWindow("Exemple1", CV_WINDOW_AUTOSIZE);
	imshow("Exemple1", img);
	waitKey(0);
	return 0;
}