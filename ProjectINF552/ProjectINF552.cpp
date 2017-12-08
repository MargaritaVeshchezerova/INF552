// ProjectINF552.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "maxflow/graph.h"
#include <iostream>
#include "DatabaseUpload.h"

using namespace cv;
using namespace std;





void testGCuts()
{
	Graph<double, double, double> g(/*estimated # of nodes*/ 2, /*estimated # of edges*/ 1);
	g.add_node(2);
	g.add_tweights(0,   /* capacities */  1,2);
	g.add_tweights(1,   /* capacities */  0, 2);
	double flow = g.maxflow();
	cout << "Flow = " << flow << endl;
	for (int i = 0; i<2; i++)
		if (g.what_segment(i) == Graph<double, double, double>::SOURCE)
			cout << i << " is in the SOURCE set" << endl;
		else
			cout << i << " is in the SINK set" << endl;
}

double maximumLikelyhood(int alpha, int i, int j, vector<Mat>& source, Mat& colorProbability) {
	
	double probability = 1.;
	for (int k= 0; k< 3; k++) {
		int index =int(((double) Scalar(source.at(alpha).at<Vec3b>(i, j)).val[k]) / colorProbability.cols);
		probability *= colorProbability.at<double>(k, index);
	}
	return(1 - probability);
}


double computeX(int alpha1, int i1, int j1,int alpha2, int i2, int j2, vector<Mat>&source) {
	if (alpha1 == alpha2) {
		return 0.;
	}
	else {
		return((norm(Scalar(source.at(alpha1).at<Vec3b>(i1, j1)) - Scalar(source.at(alpha2).at<Vec3b>(i1, j1))) + norm(Scalar(source.at(alpha1).at<Vec3b>(i2, j2)) - Scalar(source.at(alpha2).at<Vec3b>(i2, j2)))));
	}
}

double computeY(int alpha1, int i1, int j1, int alpha2, int i2, int j2, vector<Mat>&source) {
	if (alpha1 == alpha2 || i1 <= 0 || i1 >= source.at(0).rows - 1 || i2 <= 0 || i2 >= source.at(0).rows - 1 || j1 <= 0 || j1 >= source.at(0).cols - 1 || j2 <= 0 || j2 >= source.at(0).cols - 1) {
		return 0.;
	}
	else {
		//cout << "hey" << endl;
		//cout << i1 << " " << i2 << " " << j1 << " " << j2 << " " << endl;
		Scalar G_i1_alpha1 = (Scalar(source.at(alpha1).at<Vec3b>(i1 + 1, j1)) - Scalar(source.at(alpha1).at<Vec3b>(i1 - 1, j1))) / 2;
		Scalar G_i2_alpha1 = (Scalar(source.at(alpha1).at<Vec3b>(i2 + 1, j1)) - Scalar(source.at(alpha1).at<Vec3b>(i2 - 1, j1))) / 2;
		Scalar G_i1_alpha2 = (Scalar(source.at(alpha2).at<Vec3b>(i1 + 1, j1)) - Scalar(source.at(alpha2).at<Vec3b>(i1 - 1, j1))) / 2;
		Scalar G_i2_alpha2 = (Scalar(source.at(alpha2).at<Vec3b>(i2 + 1, j1)) - Scalar(source.at(alpha2).at<Vec3b>(i2 - 1, j1))) / 2;
		Scalar G_j1_alpha1 = (Scalar(source.at(alpha1).at<Vec3b>(i1 , j1 + 1)) - Scalar(source.at(alpha1).at<Vec3b>(i1 , j1 - 1))) / 2;
		Scalar G_j2_alpha1 = (Scalar(source.at(alpha1).at<Vec3b>(i2 , j1 + 1)) - Scalar(source.at(alpha1).at<Vec3b>(i2 , j1 - 1))) / 2;
		Scalar G_j1_alpha2 = (Scalar(source.at(alpha2).at<Vec3b>(i1 , j1 + 1)) - Scalar(source.at(alpha2).at<Vec3b>(i1 , j1 - 1))) / 2;
		Scalar G_j2_alpha2 = (Scalar(source.at(alpha2).at<Vec3b>(i2 , j1 + 1)) - Scalar(source.at(alpha2).at<Vec3b>(i2 , j1 - 1))) / 2;
		//cout << "nice" << endl;
		return((norm(G_i1_alpha1 - G_i1_alpha2) + norm(G_j1_alpha1 - G_j1_alpha2) + norm(G_i2_alpha1 - G_i2_alpha2) + norm(G_j2_alpha1 - G_j2_alpha2)));
	}
}

int main(int argc, char** argv)
{
	testGCuts(); //test whether graph-cut.h is well implemented


	 //nombre d'images source afin de creer la nouvelle image
	int bin = 20;
	double lambda_X = 1./30000.; 
	double lambda_Y = 1./30000;

	/*
	lambda = 10000 pour computeX

	double lambda_X = 1./15000; 
	double lambda_Y = 1./1000;
	
	double lambda_X = 1./1000;
	double lambda_Y = 1./5000;

	double lambda_X = 1./3000;
	double lambda_Y = 1./5000;

	double lambda_X = 1./3000; il y parapluie jaune et anorak rouge
	double lambda_Y = 1./3000;

	double lambda_X = 1./3000; parapluie bleu, casquette beige, blonde anorak rouge
	double lambda_Y = 1./1500;
	*/

	vector<Mat>* images = new vector<Mat>();
	loadDatabase(argv[1], *images);
	int n = images->size();

	cout << "Images have been loaded" << endl;

	//on construit les histogrammes des couleurs
	int rows = (*images)[0].rows;
	int cols = (*images)[0].cols;
	Mat colorProbability = Mat(3, bin,CV_64F); //pour chaque couleur on fait un histogramme avec bin nombre de colonnes (on divises 255 par bin)
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < bin; j++) {
			colorProbability.at<double>(i, j) = 0.; //for per-pixel maximum likelihood
		}
	}
	cout << "Beginning pixel histogram" << endl;
	//ici on doit selectionner quelles endroits on conserve je sais pas trop comment le faire, mais le prof m'a dit qu'il faut faire une sorte de brush. genre le mec glisse sa souris sur une image et on mets les coordoinnes selectionnees dans un span
	

	for (int img = 0; img < n; img++) { //au lieu de faire toutes ces boucles for, il suffit de looper sur les pixels dans le span
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				for (int b = 0; b < bin; b++) {
					if ((double)Scalar((*images)[img].at<Vec3b>(i, j)).val[0] <= (double((b + 1) * 255)) / bin) { //en fait ici il ne faut selectionner que les parties prises je pense, je me suis trompe
						colorProbability.at<double>(0, b) += 1.;
						break;
					}
					if (b == bin - 1) {
						cout << Scalar((*images).at(img).at<Vec3b>(i, j)).val[0] << endl;
					}
				}
				for (int b = 0; b < bin; b++) {
					if ((double)Scalar((*images)[img].at<Vec3b>(i, j)).val[1] <= (double((b + 1) * 255)) / bin) {
						colorProbability.at<double>(1, b) += 1.;
						break;
					}
					if (b == bin - 1) {
						cout << Scalar((*images).at(img).at<Vec3b>(i, j)).val[1] << endl;
					}
				}
				for (int b = 0; b < bin; b++) {
					if ((double)Scalar((*images).at(img).at<Vec3b>(i, j)).val[2] <= (double((b+1) * 255)) / bin) {
						colorProbability.at<double>(2, b) += 1.;
						break;
					}
					if (b == bin - 1) {
						cout << Scalar((*images).at(img).at<Vec3b>(i, j)).val[2] << endl;
					}
				}
			}
		}
	}

	double total= 0;
	for (int j = 0; j < bin; j++) {
		total += colorProbability.at<double>(0, j);
	}
	cout << total << " vs " <<  n*rows*cols<<endl;

	total = 0;
	for (int j = 0; j < bin; j++) {
		total += colorProbability.at<double>(1, j);
	}
	cout << total << " vs " <<  n*rows*cols << endl;

	total = 0;
	for (int j = 0; j < bin; j++) {
		total += colorProbability.at<double>(2, j);
	}
	cout << total << " vs " <<  n*rows*cols << endl;

	for (int color = 0; color < 3; color++) { //c'est necessaire pour maximumLikelyhood
		for (int b = 0; b < bin; b++) {
			colorProbability.at<double>(color,b) /= double(rows*cols*n); // on divise pas quantite de pixel selectionne.
			cout << "values are " << colorProbability.at<double>(color, b) <<endl;
		}
	}

	cout << "done with colorProbability" << endl;

	//alpha-extension graph-cut
	Mat Label = Mat(rows, cols, CV_32SC1); //on veut stocker les label des differents trucs; comme ca l'image est le pixel de l'image caracterise par son label
	Mat result = (*images)[0].clone(); //Nous allons stocker l'image resultante
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			Label.at<int>(i, j) = 0; // l'image resultat est d'abord initialiser a l'image zero
		}
	}
	namedWindow("previous_result", WINDOW_NORMAL | WINDOW_KEEPRATIO);
	namedWindow("current_result", WINDOW_NORMAL | WINDOW_KEEPRATIO);
	imshow("previous_result", result);
	double previous_max_flow = 0.;
	double current_max_flow = 0.;
	int lab = 1;
	while(lab <= 2 || previous_max_flow > current_max_flow) { //loop over all possible labels
		lab++;
		cout << "Start graph cut" << endl;
		for (int alpha = 0; alpha < n; alpha++) {

			Graph<double, double, double> g(/*estimated # of nodes*/ rows*cols, /*estimated # of edges*/ rows*(cols - 1) + cols*(rows - 1));
			g.add_node(rows*cols);
			for (int i = 0; i < rows; i++)
			{
				for (int j = 0; j < cols; j++)
				{
					double temp_curr = maximumLikelyhood(Label.at<int>(i, j), i, j, *images, colorProbability);
					double temp_alpha = maximumLikelyhood(alpha, i, j, *images, colorProbability);
					//cout << 100000*temp_curr << " " << 10000*temp_alpha << endl;
					/*cout << maximumLikelyhood(Label.at<int>(i, j), i, j, *images, colorProbability) << endl;
					cout << maximumLikelyhood(alpha, i, j, *images, colorProbability) << endl;*/
					g.add_tweights(i*cols + j, temp_curr, temp_alpha);
				}
			}
			for (int i = 0; i < rows; i++) {
				for (int j = 0; j < cols; j++) {
					if (i > 0) {
						//double temp1 = computeX(alpha, i, j, Label.at<int>(i, j), i - 1, j, *images); //je sais pas si ici on doit mette Label.at<int>(i,j) ... ca doit etre la que vient le label_opt = label/2
						double temp1 = lambda_Y *computeY(alpha, i, j, Label.at<int>(i, j), i - 1, j, *images) + lambda_X *computeX(alpha, i, j, Label.at<int>(i, j), i - 1, j, *images);
						//double temp2 = computeX(alpha, i, j, Label.at<int>(i-1, j), i - 1, j, *images);
						double temp2 = lambda_Y *computeY(alpha, i, j, Label.at<int>(i - 1, j), i - 1, j, *images) + lambda_X *computeX(alpha, i, j, Label.at<int>(i - 1, j), i - 1, j, *images);
						g.add_edge((i - 1)*cols + j, i*cols + j, temp2, temp1);
					}
					if (j > 0) {
						//double temp1 = computeX(alpha, i, j, Label.at<int>(i, j ), i, j - 1, *images);
						double temp1 = lambda_Y *computeY(alpha, i, j, Label.at<int>(i, j), i, j - 1, *images) + lambda_X *computeX(alpha, i, j, Label.at<int>(i, j), i, j - 1, *images);
						//double temp2 = computeX(alpha, i, j, Label.at<int>(i, j-1), i, j - 1, *images);
						double temp2 = lambda_Y *computeY(alpha, i, j, Label.at<int>(i, j - 1), i, j - 1, *images) + lambda_X *computeX(alpha, i, j, Label.at<int>(i, j - 1), i, j - 1, *images);
						g.add_edge(i*cols + j - 1, i*cols + j, temp2, temp1);
					}
				}
			}
			double flow = g.maxflow();
			if (alpha == n - 1) {
				previous_max_flow = current_max_flow;
				current_max_flow = flow;
			}
			cout << "Flow = " << flow << endl;
			for (int i = 0; i < rows*cols; i++) {
				if (g.what_segment(i) == Graph<double, double, double>::SOURCE) { //source : we keep label sink : we change label to alpha
					// cout << "huh" << endl;
					int ligne = i / cols;
					int colonne = i%cols;
					// cout << ligne<<" "<<colonne << endl;

					Label.at<int>(ligne, colonne) = alpha;
				}
			}
		}
		cout << "display end" << endl;
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				result.at<Vec3b>(i, j) = (*images).at(Label.at<int>(i, j)).at<Vec3b>(i, j);
			}
		}
		
		cout << "end graph cut" << endl;
		imshow("current_result", result);
		waitKey();
		imshow("previous_result", result);
	}
	
	imwrite("./Images/result/maximum_likelihood_Rita.jpg", result);

	waitKey(0);
	return 0;
}

