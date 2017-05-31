#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>
#include "ludo.h"

using namespace cv;
using namespace std;

int euclid(Point a, Point b) {
	return sqrt(pow((a.x - b.x), 2) + pow((a.y - b.y), 2));
}

int main(int argc, char *argv[])
{

	VideoCapture camera(0);  // ID of the (back) camera

	string inputText = argv[1] ? argv[1] : "text";

	if (!camera.isOpened())
	{
		cout << "Unable to access camera" << endl;
		return -1;
	}

	Mat frame;

	cout << "Framerate: " << camera.get(CV_CAP_PROP_FPS) << endl;

	camera.set(CV_CAP_PROP_CONTRAST, 0.5);


	LudoBoard lb = LudoBoard();

	LudoColorLogic colors[4];
	for (int i = 1; i < 5; i++) {
		colors[i - 1] = LudoColorLogic((LudoColor)i);
		lb.putOnField((LudoColor)i);
	}
	lb.setAt(6, blue, 0);
	lb.setAt(2, green, 0);
	lb.setAt(10, blue, 6);
	colors[1].isAI = true;
	int current = 0;
	lb.print();

	// Do actual image processing yay
	string imgsrc = "camerapic.jpg";
	Mat src, gray, thresh, tmp, edges;
	src = imread(imgsrc, CV_LOAD_IMAGE_COLOR);
	
	int bigCols = src.cols * 0.3;
	int bigRows = src.rows * 0.3;

	if (!src.data) {
		cout << "Could not read image!";
	}

	cvtColor(src, gray, CV_BGR2GRAY);

	GaussianBlur(gray, tmp, Size(9, 9), 2, 2);

	Canny(gray, edges, 50, 200, 3);

	threshold(tmp, thresh, 105, 255, 1);
	/*imshow("Img", thresh);
	waitKey(1);*/

	vector<vector<Point>> countours;

	findContours(edges, countours, 1, 2);


	vector<vector<Point>> lel;
	for (int i = 0; i < countours.size(); i++) {
		vector<Point> cnt = countours.at(i);
		vector<Point> res;
		approxPolyDP(cnt, res, 0.01 * arcLength(cnt, true), true);
		if (res.size() == 4 && euclid(res[0], res[1]) > bigCols) {
			lel.push_back(cnt);
			drawContours(src, countours, i, (0, 0, 255), 1);
			//break;
		}
	}


	while (true) {
		// Do something lol;
		imshow("Img", src);
		waitKey(1);
	}


	//while (true)
	//{

	//	camera.read(frame);

	//	// rectangle(frame, Point(20, 20), Point(600, 400), Scalar(0, 255, 0, 0), 3);

	//	// putText(frame, inputText, Point(100, 150), FONT_HERSHEY_SIMPLEX, 2, Scalar(255, 0, 0, 0), 2);

	//	imshow("Camera", frame);

	//	signed char keyPressed = waitKey(30);
	//	if (keyPressed == 'P' || keyPressed == 'p') {
	//		imwrite("camerapic.jpg", frame);
	//		inputText = "pic saved!";
	//	}
	//	else if (keyPressed == '\r' || keyPressed == '\n') {
	//		break;
	//	}
	//}

	return 0;
}
