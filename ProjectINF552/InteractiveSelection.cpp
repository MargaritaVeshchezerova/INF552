#include"InteractiveSelection.h"

const int DefaultLabel = -1;

class ExchangedData
{
public:
	Mat image;
	Mat_<int> labels;
	int label;
	int* radius;
	ExchangedData(Mat& img, Mat_<int>& lbs, int lb)
	{
		image = img;
		labels = lbs;
		label = lb;
		radius = new int(5);
	}
	~ExchangedData()
	{

	}
};

//How to get mouse position
void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
	Mat image = ((ExchangedData*)userdata)->image;
	Mat_<int> labels = ((ExchangedData*)userdata)->labels;
	int label = ((ExchangedData*)userdata)->label;
	int radius = *((ExchangedData*)userdata)->radius;

	if (flags == (EVENT_FLAG_LBUTTON))
	{
		circle(image, Point(x, y), radius, Scalar(255, 0, 0), -1, 8);
		circle(labels, Point(x, y), radius, label, -1, 8);
		cout << "Left mouse button is clicked - position (" << x << ", " << y << ") radius: " << radius << endl;
	}
}


Mat_<int>* labeling(vector<Mat>& dataset, int size)
{
	int n = size;
	if (n == 0)
		return NULL;

	int rows = dataset[0].rows;
	int cols = dataset[0].cols;

	Mat_<int>* labels = new Mat_<int>(rows, cols, DefaultLabel);

	vector<ExchangedData*>* collectedData = new vector<ExchangedData*>();
	for (int i = 0; i < n; i++)
	{
		collectedData->push_back(new ExchangedData(dataset[i], *labels, i));
	}

	for (int i = 0; i < n; i++)
	{
		namedWindow("Window: " + i, WINDOW_NORMAL | WINDOW_KEEPRATIO);
		createTrackbar("Brush radius: " + i, "Window: " + i, (*collectedData)[i]->radius, 20, NULL, NULL);
		setMouseCallback("Window: " + i, CallBackFunc, (*collectedData)[i]);
	}

	while (1)
	{
		for (int i = 0; i < n; i++)
		{
			setTrackbarPos("Brush radius: " + i, "Window: " + i, *(*collectedData)[i]->radius);
			imshow("Window: " + i, dataset[i]);
		}
		if (waitKey(20) == 27)
		{
			destroyAllWindows();
			break;
		}
	}

	for (int i = 0; i < n; i++)
	{
		delete (*collectedData)[i];
	}

	delete collectedData;

	return labels;
}

int main()
{
	Mat img = imread("C:\\Users\\Margarita\\Downloads\\Rita.jpg");
	Mat img1 = imread("C:\\Users\\Margarita\\Downloads\\chantage.jpg");
	vector<Mat> vM;
	vM.push_back(img);
	vM.push_back(img1);
	Mat_<int>* label = labeling(vM, 2);
	return 0;
}




