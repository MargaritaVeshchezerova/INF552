// ProjectINF552.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "maxflow2/graph.h"
#include <iostream>

using namespace cv;
using namespace std;





void testGCuts()
{
	Graph<int, int, int> g(/*estimated # of nodes*/ 2, /*estimated # of edges*/ 1);
	g.add_node(2);
	g.add_tweights(0,   /* capacities */  1.2,0);
	g.add_tweights(1,   /* capacities */  0, 1.2);
	g.add_edge(0, 1,    /* capacities */  3, 3.123);
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
	return(norm(Scalar(source.at(alpha).at<Vec3b>(i, j) - source.at(Label.at<int>(i, j)).at<Vec3b>(i, j))));
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
	
	double probability = 1.;
	for (int i = 0; i < 3; i++) {
		int index =int(((float) Scalar(source.at(alpha).at<Vec3b>(i, j)).val[i]) / colorProbability.cols);
		probability *= colorProbability.at<float>(i, index);
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
		return((norm(Scalar(source.at(alpha1).at<Vec3b>(i1, j1)) - Scalar(source.at(alpha2).at<Vec3b>(i1, j1))) + norm(Scalar(source.at(alpha1).at<Vec3b>(i2, j2)) - Scalar(source.at(alpha2).at<Vec3b>(i2, j2)))));
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


struct MouseParams
{
	Mat* img;
	int* draw;
	MouseParams(Mat* img1 , int* draw1) {
		img = img1;
		draw = draw1;
	}
};

void draw_circle(int event, int x, int y, int, void* param) {
	MouseParams* mp = (MouseParams*)param;
	Mat* img = mp->img;
	if (event == CV_EVENT_MOUSEMOVE)
	{
		if (*mp->draw == 1) {
			circle(*img, Point(x, y), 10, Scalar(255, 255, 255),-1);
		}
	}
	else if (event == CV_EVENT_LBUTTONDOWN) {
		*mp->draw = 1;
	}
	else if (event = CV_EVENT_LBUTTONUP) {
		*mp->draw = 0;
	}
}


int main()
{
	vector<Mat> source;
	vector<int> drawing;
	vector<MouseParams> mps;
	
	int n = 5; //nombre d'images source afin de creer la nouvelle image
	int bin = 20;

	for (int i = 0; i < n; i++) {
		String location = "./Images/cathedral/d00" + to_string(i+1) + ".jpg";
		Mat img = imread(location);
		//namedWindow("d00" + to_string(i+1));
		//imshow("d00" + to_string(i+1), img);
		source.push_back(img);
		drawing.push_back(0);
	}

	/*for (int i = 0; i < n; i++) {
		MouseParams* mp = new MouseParams(&source.at(i), &drawing.at(i));
		setMouseCallback("d00" + to_string(i + 1), draw_circle, (void*)mp);
	}*/

	//for (int i = 0; i < 300; i++) {
	//	for (int j = 0; j < 300; j++) {
	//		source.at(0).at<Vec3b>(i, j) = Vec3b(255, 255, 255);
	//		imshow("d001", source.at(0));
	//	}
	//}

	/*while (waitKey(20) != 27)
	{
		for (int i = 0; i < n; i++) {
			imshow("d00" + to_string(i + 1), source.at(i));
		}
	}*/

	cout << "Images have been loaded" << endl;




	int rows = source.at(0).rows;
	int cols = source.at(0).cols;
	Mat colorProbability(3, bin,CV_32F); //pour chaque couleur on fait un histogramme avec bin nombre de colonnes (on divises 255 par bin)
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < bin; j++) {
			colorProbability.at<float>(i, j) = 0.; //for per-pixel maximum likelihood
		}
	}
	cout << "Beginning pixel histogram" << endl;
	//ici on doit selectionner quelles endroits on conserve je sais pas trop comment le faire, mais le prof m'a dit qu'il faut faire une sorte de brush. genre le mec glisse sa souris sur une image et on mets les coordoinnes selectionnees dans un span
	

	for (int img = 0; img < n; img++) { //au lieu de faire toutes ces boucles for, il suffit de looper sur les pixels dans le span
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				for (int b = 0; b < bin; b++) {
					//cout << source.at(img).at<Vec3b>(i, j) << endl;
					//cout << ((float)Scalar(source.at(img).at<Vec3b>(i, j)).val[0] < 255) << endl;
					//cout << ((float)Scalar(source.at(img).at<Vec3b>(i, j)).val[0] < 0) << endl;

					//cout << Scalar(source.at(img).at<Vec3b>(i, j)).val[1] << endl;
					//cout << Scalar(source.at(img).at<Vec3b>(i, j)).val[2] << endl;
					//cout << "///////////////////////////////////////////////" << endl;
					if ((float)Scalar(source.at(img).at<Vec3b>(i, j)).val[0] <= (float((b + 1) * 255)) / bin) { //en fait ici il ne faut selectionner que les parties prises je pense, je me suis trompe
						colorProbability.at<float>(0, b) += 1.;
						break;
					}
					if (b == bin - 1) {
						cout << Scalar(source.at(img).at<Vec3b>(i, j)).val[0] << endl;
					}
				}
				for (int b = 0; b < bin; b++) {
					if ((float)Scalar(source.at(img).at<Vec3b>(i, j)).val[1] <= (float((b + 1) * 255)) / bin) {
						colorProbability.at<float>(1, b) += 1.;
						break;
					}
					if (b == bin - 1) {
						cout << Scalar(source.at(img).at<Vec3b>(i, j)).val[1] << endl;
					}
				}
				for (int b = 0; b < bin; b++) {
					if ((float)Scalar(source.at(img).at<Vec3b>(i, j)).val[2] <= (float((b+1) * 255)) / bin) {
						colorProbability.at<float>(2, b) += 1.;
						break;
					}
					if (b == bin - 1) {
						cout << Scalar(source.at(img).at<Vec3b>(i, j)).val[2] << endl;
					}
				}
			}
		}
	}

	float total= 0;
	for (int j = 0; j < bin; j++) {
		total += colorProbability.at<float>(0, j);
	}
	cout << total << " vs " << 3 * n*rows*cols<<endl;

	total = 0;
	for (int j = 0; j < bin; j++) {
		total += colorProbability.at<float>(1, j);
	}
	cout << total << " vs " << 3 * n*rows*cols << endl;

	total = 0;
	for (int j = 0; j < bin; j++) {
		total += colorProbability.at<float>(2, j);
	}
	cout << total << " vs " << 3 * n*rows*cols << endl;

	for (int color = 0; color < 3; color++) { //c'est necessaire pour maximumLikelyhood
		for (int b = 0; b < bin; b++) {
			colorProbability.at<float>(color,b) /= float(rows*cols*n); // on divise pas quantite de pixel selectionne.
			cout << "values are " << colorProbability.at<float>(color, b) <<endl;
		}
	}

	cout << "done with colorProbability" << endl;

	Mat Label(rows, cols, CV_32SC1); //on veut stocker les label des differents trucs; comme ca l'image est le pixel de l'image caracterise par son label
	Mat result = source.at(0).clone(); //Nous allons stocker l'image resultante
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			Label.at<int>(i, j) = 0; // l'image resultat est d'abord initialiser a l'image zero
		}
	}
	cout << "nice one" << endl;

	for (int alpha = 1; alpha < n;alpha ++) { //loop over all possible labels
		Graph<int, int, int> g(/*estimated # of nodes*/ rows*cols, /*estimated # of edges*/ rows*(cols-1) + cols*(rows-1));
		g.add_node(rows*cols);
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				float temp_curr = maximumLikelyhood(Label.at<int>(i, j), i, j, source, colorProbability) - 0.99;
				float temp_alpha = maximumLikelyhood(alpha, i, j, source, colorProbability)- 0.99;
				cout << temp_curr << " " << temp_alpha << endl;
				/*cout << maximumLikelyhood(Label.at<int>(i, j), i, j, source, colorProbability) << endl;
				cout << maximumLikelyhood(alpha, i, j, source, colorProbability) << endl;*/
				g.add_tweights(i*cols + j,10000*temp_curr, 10000*temp_alpha);
			}
		}
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				if (i > 0) {
					float temp1 = computeX(alpha, i, j, Label.at<int>(i, j), i - 1, j, source); //je sais pas si ici on doit mette Label.at<int>(i,j) ... ca doit etre la que vient le label_opt = label/2
					float temp2 = computeX(alpha, i, j, Label.at<int>(i-1, j), i - 1, j, source);
					g.add_edge((i - 1)*cols + j, i*cols + j, 10* temp1,10* temp2);
				}
				if (j > 0) {
					float temp1 = computeX(alpha, i, j, Label.at<int>(i, j ), i, j - 1, source);
					float temp2 = computeX(alpha, i, j, Label.at<int>(i, j-1), i, j - 1, source);

					g.add_edge(i*cols + j - 1, i*cols + j, temp1, temp2);
				}
			}
		}
		int flow = g.maxflow();
		cout << "Flow = " << flow << endl;
		for (int i = 0; i < rows*cols; i++) {
			if (g.what_segment(i) == Graph<int, int, int>::SINK) { //source : we keep label sink : we change label to alpha
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
			result.at<Vec3b>(i, j) = source.at(Label.at<int>(i, j)).at<Vec3b>(i, j);
		}
	}
	namedWindow("result");
	imshow("result", result);
	//
	//for (int i = 0; i < rows; i++) {
	//	for (int j = 0; j < cols; j++) {
	//		if (Label.at<int>(i, j) != 0) {
	//			cout << Label.at<int>(i, j) << endl;

	//		}
	//	}
	//}

	testGCuts();
	namedWindow("result");

	waitKey(0);
	return 0;
}

