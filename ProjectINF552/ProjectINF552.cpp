// ProjectINF552.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "maxflow/graph.h"
#include <iostream>

using namespace cv;
using namespace std;





void testGCuts()
{
	Graph<int, int, int> g(/*estimated # of nodes*/ 2, /*estimated # of edges*/ 1);
	g.add_node(2);
	g.add_tweights(0,   /* capacities */  1, 5);
	g.add_tweights(1,   /* capacities */  6, 1);
	g.add_edge(0, 1,    /* capacities */  4, 3);
	int flow = g.maxflow();
	cout << "Flow = " << flow << endl;
	for (int i = 0; i<2; i++)
		if (g.what_segment(i) == Graph<int, int, int>::SOURCE)
			cout << i << " is in the SOURCE set" << endl;
		else
			cout << i << " is in the SINK set" << endl;
}

int main()
{
	vector<Mat> source;
	
	int n = 5; //nombre d'images source afin de creer la nouvelle image

	for (int i = 0; i < n; i++) {
		String location = "./Images/cathedral/d00" + to_string(i+1) + ".jpg";
		cout << location << endl;
		Mat img = imread(location);
		//namedWindow("d00" + to_string(i+1));
		//imshow("d00" + to_string(i+1), img);
	}
	testGCuts();
	//namedWindow("result");

	waitKey(0);
	return 0;
}

