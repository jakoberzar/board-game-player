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

void on_trackbar(int, void*)
{
}


Vec3b averageCirclePixel(Mat src, Point center, int radius) {
	int startY = center.y - radius + 1;
	int endY = center.y + radius - 1;
	Vec<unsigned long long, 3> sum = 1;
	unsigned long long n = 0;

	for (int i = startY; i <= endY; i++) {
		int yDiff = abs(center.y - i);
		int distance = floor(sqrt(pow(radius, 2) - pow(yDiff, 2)));
		for (int j = center.x - distance + 1; j <= center.x + distance - 1; j++) {
			sum += src.at<Vec3b>(i, j);
			n++;
		}
	}

	char b, g, r;
	b = sum.val[0] / n;
	g = sum.val[1] / n;
	r = sum.val[2] / n;

	return Vec3b(b, g, r);
}

bool isOfColor(Mat board, Vec3f circle) {
	Vec3f avg = averageCirclePixel(board, Point(circle[0], circle[1]), circle[2]);
	return (avg.val[0] + avg.val[1] + avg.val[2] >= 200);
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

	threshold(gray, thresh, 105, 255, 1);
	/*imshow("Img", thresh);
	waitKey(1);*/

	vector<vector<Point>> countours;

	findContours(edges, countours, 1, 2);


	vector<Point> outside;
	for (int i = 0; i < countours.size(); i++) {
		vector<Point> cnt = countours.at(i);
		vector<Point> res;
		approxPolyDP(cnt, res, 0.01 * arcLength(cnt, true), true);
		if (res.size() == 4 && euclid(res[0], res[1]) > bigCols) {
			outside = res;
			drawContours(src, countours, i, (0, 0, 255), 1);
			break;
		}
	}

	Rect basicBounds = boundingRect(outside);


	// Get the board rectangle only
	Mat boardSrc = src(basicBounds).clone();

	Mat M = getRotationMatrix2D(Point(boardSrc.cols / 2, boardSrc.rows / 2), -2.8, 1);
	warpAffine(boardSrc, boardSrc, M, Size(boardSrc.rows, boardSrc.cols));
	// TODO: RESIZE TO ALWAYS THE SAME SIZE, CALCULATE ROTATION

	// Get the circles from the picture
	cvtColor(boardSrc, gray, CV_BGR2GRAY);
	//Canny(gray, edges, 50, 200, 3);

	vector<Vec3f> circles;
	HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 1, 5, 200, 20, 5, 15);
	Mat boardBlackCircles = boardSrc.clone();
	for (size_t i = 0; i < circles.size(); i++)
	{
		Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		int radius = cvRound(circles[i][2]);
		// draw the circle center
		//circle(boardSrc, center, 1, Scalar(0, 255, 0), -1, 8, 0);
		// draw the circle outline
		circle(boardBlackCircles, center, radius, Scalar(0, 0, 0), 1, 8, 0);
	}

	// Detect the colors of circles
	Mat boardHSV, whiteBoard, redBoard, redBoard1, redBoard2, greenBoard, yellowBoard, blueBoard;
	cvtColor(boardBlackCircles, boardHSV, COLOR_BGR2HSV);
	int tolerance = 125;
	inRange(boardHSV, Scalar(80, 0, 145), Scalar(180, 150, 255), whiteBoard);
	inRange(boardHSV, Scalar(0, 100, 100), Scalar(10, 255, 255), redBoard1);
	inRange(boardHSV, Scalar(125, 100, 100), Scalar(180, 255, 255), redBoard2);
	bitwise_or(redBoard1, redBoard2, redBoard);
	inRange(boardHSV, Scalar(20, 100, 100), Scalar(35, 255, 255), yellowBoard);
	inRange(boardHSV, Scalar(65, 20, 135), Scalar(100, 255, 255), greenBoard);
	inRange(boardHSV, Scalar(105, 150, 0), Scalar(135, 255, 255), blueBoard);

	Mat whiteBBGR, redBBGR, greenBBGR, yellowBBGR, blueBBGR;
	cvtColor(whiteBoard, whiteBBGR, CV_GRAY2BGR);
	cvtColor(redBoard, redBBGR, CV_GRAY2BGR);
	cvtColor(greenBoard, greenBBGR, CV_GRAY2BGR);
	cvtColor(yellowBoard, yellowBBGR, CV_GRAY2BGR);
	cvtColor(blueBoard, blueBBGR, CV_GRAY2BGR);

	vector<Vec3f> circles_white, circles_red, circles_yellow, circles_green, circles_blue;
	for (int i = 0; i < circles.size(); i++) {
		Vec3f c = circles[i];
		Scalar s;
		if (isOfColor(redBBGR, c)) {
			circles_red.push_back(c);
			cout << "red";
			s = Scalar(0, 0, 255);
		}
		else if (isOfColor(greenBBGR, c)) {
			circles_green.push_back(c);
			s = Scalar(0, 255, 0);
		}
		else if (isOfColor(blueBBGR, c)) {
			circles_blue.push_back(c);
			s = Scalar(255, 0, 0);
		}
		else if (isOfColor(yellowBBGR, c)) {
			circles_yellow.push_back(c);
			s = Scalar(0, 255, 255);
		}
		else if (isOfColor(whiteBBGR, c)) {
			circles_white.push_back(c);
			s = Scalar(0, 0, 0);
		}
		else {
			cout << "Unknown circle..." << endl;
			s = Scalar(0, 0, 0);
		}
		Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		int radius = cvRound(circles[i][2]);
		circle(boardSrc, center, radius, s, 2, 8, 0);
	}

	// TODO: Indentify which circles mean what



	//const int slider_hue_max = 180;
	//const int slider_sat_max = 255;
	//const int slider_val_max = 255;
	//int slider_hue = 0;
	//int slider_sat = 0;
	//int slider_val = 255 - tolerance;
	//int slider_up_hue = 180;
	//int slider_up_sat = 125;
	//int slider_up_val = 255;

	while (true) {
		/*inRange(boardHSV, Scalar(slider_hue, slider_sat, slider_val), Scalar(slider_up_hue, slider_up_sat, slider_up_val), greenBoard);
		createTrackbar("H", "Img", &slider_hue, slider_hue_max, on_trackbar);
		createTrackbar("H2", "Img", &slider_up_hue, slider_hue_max, on_trackbar);
		createTrackbar("S", "Img", &slider_sat, slider_sat_max, on_trackbar);
		createTrackbar("S2", "Img", &slider_up_sat, slider_sat_max, on_trackbar);
		createTrackbar("V", "Img", &slider_val, slider_val_max, on_trackbar);
		createTrackbar("V2", "Img", &slider_up_val, slider_val_max, on_trackbar);
		Mat display;
		boardSrc.copyTo(display, whiteBoard);
		imshow("Img", display);*/
		imshow("src", boardSrc);
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
