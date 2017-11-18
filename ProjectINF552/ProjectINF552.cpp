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

double designatedColor(int alpha, int i, int j, vector<Mat>& source, Mat& Label)
{
	norm(Scalar(source.at(alpha).at<Vec3b>(i, j) - source.at(Label.at<int>(i, j)).at<Vec3b>(i, j)));
}

double computeDesignatedColorDataPenalty(int alpha, vector<Mat>& source, Mat& Label)
{
	double dataPenalty = 0.;

	Scalar Poisson(255, 255, 255, 0); //la cathedrale est blanche
	Scalar Eau(0, 0, 0, 0); // les hommes sont noirs, ca ne marchera pas car pas tous les hommes sont noirs, c'est pour tester le truc

	for (int i = 0; i < Label.rows; i++) {
		for (int j = 0; j < Label.cols; j++) {
			dataPenalty += designatedColor(alpha, i, j, source, Label);
		}
	}
	return dataPenalty;
}

double maximumLikelyhood(int alpha, int i, int j, vector<Mat>& source, Mat& colorProbability) {
	
	double probability = 0.;
	for (int i = 0; i < 3; i++) {
		int index =int(float(source.at(alpha).at<Vec3b>(i, j).val[i]) / colorProbability.cols);
		probability += colorProbability.at<float>(i, index);
	}
	return(1 - probability);
}

double computeMaximumLikelyhood(int alpha, vector<Mat>& source, Mat&Label, Mat& colorProbability) {
	double dataPenalty = 0.;

	for (int i = 0; i < Label.rows; i++) {
		for (int j = 0; j < Label.cols; j++) {
			dataPenalty += maximumLikelyhood(alpha, i, j, source, colorProbability);
		}
	}
	return dataPenalty;
}

double computeX(int alpha1, int i1, int j1,int alpha2, int i2, int j2, vector<Mat>&source) {
	if (alpha1 == alpha2) {
		return 0.;
	}
	else {
		return(norm(Scalar(source.at(alpha1).at<Vec3b>(i1, j1)) - Scalar(source.at(alpha2).at<Vec3b>(i1, j1))) + norm(Scalar(source.at(alpha1).at<Vec3b>(i2, j2)) - Scalar(source.at(alpha2).at<Vec3b>(i2, j2))));
	}
}

double computeY(int alpha1, int i1, int j1, int alpha2, int i2, int j2, vector<Mat>&source) {
	if (alpha1 == alpha2) {
		return 0.;
	}
	else {
		Scalar G_i1_alpha1 = (i1 == 0 || i1 == source.at(0).rows - 1 ? 0. : (Scalar(source.at(alpha1).at<Vec3b>(i1 + 1, j1)) - Scalar(source.at(alpha1).at<Vec3b>(i1 - 1, j1))) / 2);
		Scalar G_i2_alpha1 = (i2 == 0 || i2 == source.at(0).rows - 1 ? 0. : (Scalar(source.at(alpha1).at<Vec3b>(i2 + 1, j1)) - Scalar(source.at(alpha1).at<Vec3b>(i2 - 1, j1))) / 2);
		Scalar G_i1_alpha2 = (i1 == 0 || i1 == source.at(0).rows - 1 ? 0. : (Scalar(source.at(alpha2).at<Vec3b>(i1 + 1, j1)) - Scalar(source.at(alpha2).at<Vec3b>(i1 - 1, j1))) / 2);
		Scalar G_i2_alpha2 = (i2 == 0 || i2 == source.at(0).rows - 1 ? 0. : (Scalar(source.at(alpha2).at<Vec3b>(i2 + 1, j1)) - Scalar(source.at(alpha2).at<Vec3b>(i2 - 1, j1))) / 2);
		Scalar G_j1_alpha1 = (j1 == 0 || j1 == source.at(0).cols - 1 ? 0. : (Scalar(source.at(alpha1).at<Vec3b>(i1 , j1 + 1)) - Scalar(source.at(alpha1).at<Vec3b>(i1 , j1 - 1))) / 2);
		Scalar G_j2_alpha1 = (j2 == 0 || j2 == source.at(0).cols - 1 ? 0. : (Scalar(source.at(alpha1).at<Vec3b>(i2 , j1 + 1)) - Scalar(source.at(alpha1).at<Vec3b>(i2 , j1 - 1))) / 2);
		Scalar G_j1_alpha2 = (j1 == 0 || j1 == source.at(0).cols - 1 ? 0. : (Scalar(source.at(alpha2).at<Vec3b>(i1 , j1 + 1)) - Scalar(source.at(alpha2).at<Vec3b>(i1 , j1 - 1))) / 2);
		Scalar G_j2_alpha2 = (j2 == 0 || j2 == source.at(0).cols - 1 ? 0. : (Scalar(source.at(alpha2).at<Vec3b>(i2 , j1 + 1)) - Scalar(source.at(alpha2).at<Vec3b>(i2 , j1 - 1))) / 2);
		return(norm(G_i1_alpha1 - G_i1_alpha2) + norm(G_j1_alpha1 - G_j1_alpha2) + norm(G_i2_alpha1 - G_i2_alpha2) + norm(G_j2_alpha1 - G_j2_alpha2));
	}
}

int main()
{
	vector<Mat> source;
	
	int n = 5; //nombre d'images source afin de creer la nouvelle image
	int bin = 20;

	for (int i = 0; i < n; i++) {
		String location = "./Images/cathedral/d00" + to_string(i+1) + ".jpg";
		cout << location << endl;
		Mat img = imread(location);
		cout << img.at<Vec3b>(3, 3) << endl;
		//namedWindow("d00" + to_string(i+1));
		//imshow("d00" + to_string(i+1), img);
		source.at(i) = img;
	}
	int rows = source.at(0).rows;
	int cols = source.at(0).cols;
	Mat colorProbability(3, bin,CV_32F); //pour chaque couleur on fait un histogramme avec bin nombre de colonnes (on divises 255 par bin)
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			colorProbability.at<float>(i, j) = 0;
		}
	}

	//ici on doit selectionner quelles endroits on conserve je sais pas trop comment le faire, mais le prof m'a dit qu'il faut faire une sorte de brush. genre le mec glisse sa souris sur une image et on mets les coordoinnes selectionnees dans un span
	

	for (int img = 0; img < n; img++) { //au lieu de faire toutes ces boucles for, il suffit de looper sur les pixels dans le span
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				for (int b = 0; b < bin; b++) {
					if (source.at(img).at<Vec3b>(i, j).val[0] <= (float(b * 255)) / bin) { //en fait ici il ne faut selectionner que les parties prises je pense, je me suis trompe
						colorProbability.at<float>(0, b) += 1.;
						break;
					}
				}
				for (int b = 0; b < bin; b++) {
					if (source.at(img).at<Vec3b>(i, j).val[1] <= (float(b * 255)) / bin) {
						colorProbability.at<float>(1, b) += 1.;
						break;
					}
				}
				for (int b = 0; b < bin; b++) {
					if (source.at(img).at<Vec3b>(i, j).val[2] <= (float(b * 255)) / bin) {
						colorProbability.at<float>(2, b) += 1.;
						break;
					}
				}
			}
		}
	}


	for (int color = 0; color < 3; color++) { //c'est necessaire pour maximumLikelyhood
		for (int b = 0; b < bin; b++) {
			colorProbability.at<float>(color,b) /= float(3*rows*cols*n); // on divise pas quantite de pixel selectionne.
		}
	}

	Mat Label(rows, cols, CV_16U); //on veut stocker les label des differents trucs; comme ca l'image est le pixel de l'image caracterise par son label
	Mat result(rows, cols, CV_32F); //Nous allons stocker l'image resultante
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			Label.at<int>(i, j) = 0; // l'image resultat est d'abord initialiser a l'image zero
		}
	}


	for (int alpha = 0; alpha < n;alpha ++) { //loop over all possible labels
		Graph<int, int, int> g(/*estimated # of nodes*/ rows*cols, /*estimated # of edges*/ rows*(cols-1) + cols*(rows-1));
		g.add_node(rows*cols);
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				g.add_tweights(i*cols + j, maximumLikelyhood(Label.at<int>(i, j), i, j, source, colorProbability), maximumLikelyhood(alpha, i, j, source, colorProbability));
			}
		}
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				if (i > 0) {
					float temp = computeX(alpha, i, j, Label.at<int>(i - 1, j), i - 1, j, source); //je sais pas si ici on doit mette Label.at<int>(i,j) ... ca doit etre la que vient le label_opt = label/2
					g.add_edge((i - 1)*cols + j, i*cols + j, temp, temp);
				}
				if (j > 0) {
					float temp = computeX(alpha, i, j, Label.at<int>(i, j - 1), i, j - 1, source);
					g.add_edge(i*cols + j - 1, i*cols + j, temp, temp);
				}
			}
		}
		int flow = g.maxflow();
		cout << "Flow = " << flow << endl;
		for (int i = 0; i < 2; i++) {
			if (g.what_segment(i) == Graph<int, int, int>::SINK) { //source : we keep label sink : we change label to alpha
				int ligne = i / cols;
				int colonne = i%cols;
				Label.at<int>(ligne, colonne) = alpha;
			}
		}
	}

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			result.at<Vec3b>(i, j) = source.at(Label.at<int>(i, j)).at<Vec3b>(i, j);
		}
	}
	namedWindow("result");
	imshow("result", result);


	//testGCuts();
	//namedWindow("result");

	waitKey(0);
	return 0;
}

