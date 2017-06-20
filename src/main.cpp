#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>
#include "ludo.h"

using namespace cv;
using namespace std;

// Function declarations
Vec2i getGridPosition(Vec3f circle);
char getGridIndex(Vec2i gridPosition, int *index);

// Color masks
Mat boardHSV, whiteBoard, redBoard, redBoard1, redBoard2, greenBoard, yellowBoard, blueBoard;
Mat whiteBBGR, redBBGR, greenBBGR, yellowBBGR, blueBBGR;
VideoCapture camera;  // ID of the (back) camera
bool cameraMode = false;
int par1, par2, par3, par4;

// Sliders
//const int slider_hue_max = 180;
//const int slider_sat_max = 255;
//const int slider_val_max = 255;
//int slider_hue = 100;
//int slider_sat = 150;
//int slider_val = 0;
//int slider_up_hue = 135;
//int slider_up_sat = 225;
//int slider_up_val = 255;

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
	Point2i p1(cvRound(circle[0] - circle[2]), cvRound(circle[1] - circle[2]));
	Point2i p2(cvRound(circle[0] + circle[2]), cvRound(circle[1] + circle[2]));
	Rect r = Rect(p1, p2);
	Mat mask, src, gray, tmp, edges;
	mask = Mat(board.rows, board.cols, board.type());
	mask.setTo(cv::Scalar(0, 0, 0));

	int index;
	Vec2i grid = getGridPosition(circle);
	char pos = getGridIndex(grid, &index);
	//int radius = pos != 'h' ? cvRound(circle[2]) - 6: 5;
	int radius = 5;
	cv::circle(mask, Point(cvRound(circle[0]), cvRound(circle[1])), radius, Scalar(255, 255, 255), -1, 8, 0);

	(board).copyTo(src, mask);
	src = src(r);

	cvtColor(src, gray, CV_BGR2GRAY);
	GaussianBlur(gray, tmp, Size(3, 3), 3, 3);
	Canny(tmp, edges, 30, 200, 3);

	vector<vector<Point>> countours;
	findContours(edges, countours, 0, 2);

	for (int i = 0; i < countours.size(); i++) {
		vector<Point> cnt = countours.at(i);
		if (cnt.size() > 15) {
			return true;
		}
	}

	return false;
}

// Returns the index of pos;
// 0 1
// 3 2
int circleOutside(Mat board, Vec3f circle) {
	int up = 0.35 * board.rows;
	int left = 0.35 * board.cols;
	int down = board.rows - up;
	int right = board.cols - left;
	Point p = Point(circle[0], circle[1]);

	if (p.x < left && p.y < up) {
		return 0;
	}
	if (p.x > right && p.y < up) {
		return 1;
	}
	if (p.x < left && p.y > down) {
		return 2;
	}
	if (p.x > right && p.y > down) {
		return 3;
	}
	return -1;
}

// Returns the index of home;
//   1
// 0   2
//   3
int circleHome(Mat board, Vec3f circle) {
	int rowSize = 0.2 * board.rows;
	int rowUpper = 0.4 * board.rows;
	int rowLower = rowUpper + rowSize;

	int colSize = 0.2 * board.cols;
	int colUpper = 0.4 * board.cols;
	int colLower = colUpper + colSize;

	Point p = Point(circle[0], circle[1]);

	if (p.x < colLower && p.y > rowUpper && p.y < rowLower) {
		return 0;
	}
	if (p.x > colUpper && p.y > rowUpper && p.y < rowLower) {
		return 2;
	}
}

void colorCirlces(Mat board, vector<Vec3f> circles, int thickness = -1) {
	for (int i = 0; i < circles.size(); i++) {
		Vec3f c =  circles[i];
		Scalar s;
		if (isOfColor(redBBGR, c)) {
			s = Scalar(0, 0, 255);
		}
		else if (isOfColor(greenBBGR, c)) {
			s = Scalar(0, 255, 0);
		}
		else if (isOfColor(blueBBGR, c)) {
			s = Scalar(255, 0, 0);
		}
		else if (isOfColor(yellowBBGR, c)) {
			s = Scalar(0, 255, 255);
		}
		//else if (isOfColor(whiteBBGR, c)) {
		//	s = Scalar(0, 0, 0);
		//}
		else {
			s = Scalar(0, 0, 0);
		}
		Point center(cvRound(c[0]), cvRound(c[1]));
		int radius = cvRound(c[2]);
		circle(board, center, radius, s, thickness, 8, 0);
	}
}

void initColorMasks(Mat boardCircles) {
	cvtColor(boardCircles, boardHSV, COLOR_BGR2HSV);
	inRange(boardHSV, Scalar(75, 0, 135), Scalar(130, 115, 255), whiteBoard);
	inRange(boardHSV, Scalar(0, 70, 100), Scalar(10, 255, 255), redBoard1);
	inRange(boardHSV, Scalar(135, 70, 100), Scalar(180, 255, 255), redBoard2);
	bitwise_or(redBoard1, redBoard2, redBoard);
	inRange(boardHSV, Scalar(20, 90, 100), Scalar(35, 255, 255), yellowBoard);
	inRange(boardHSV, Scalar(45, 70, 00), Scalar(90, 255, 255), greenBoard);
	inRange(boardHSV, Scalar(100, 80, 0), Scalar(135, 255, 255), blueBoard);

	greenBBGR.setTo(Scalar(0, 0, 0));
	redBBGR.setTo(Scalar(0, 0, 0));
	yellowBBGR.setTo(Scalar(0, 0, 0));
	blueBBGR.setTo(Scalar(0, 0, 0));
	whiteBBGR.setTo(Scalar(0, 0, 0, 0));

	boardCircles.copyTo(whiteBBGR, whiteBoard);
	boardCircles.copyTo(redBBGR, redBoard);
	boardCircles.copyTo(greenBBGR, greenBoard);
	boardCircles.copyTo(yellowBBGR, yellowBoard);
	boardCircles.copyTo(blueBBGR, blueBoard);
}

void seperateOutsideCircles(Mat board, vector<Vec3f> all_circles, vector<Vec3f> *outside_circles, vector<Vec3f> *remaining) {
	int index_del = 0; // Adjust index for deleted members
	for (int i = 0; i < all_circles.size(); i++) {
		Vec3f c = all_circles[i];
		if (circleOutside(board, c) != -1) {
			if (remaining->size() > i + index_del) {
				remaining->erase(remaining->begin() + (i + index_del));
			}
			index_del -= 1;
			(*outside_circles).push_back(c);
		}
	}
}

Vec2i getGridPosition(Vec3f circle) {
	double singleWidth = 500.0 / 11.0;
	return Vec2i(circle[0] / singleWidth, circle[1] / singleWidth);
}

// Returns g if it is on grid, h if on home, null if not
char getGridIndex(Vec2i gridPosition, int *index) {
	int x = gridPosition[0];
	int y = gridPosition[1];

	// First, check if it is not outside the grid
	if ((x < 4 || x > 6) && (y < 4 || y > 6) || (x == 5 && y == 5)) {
		return '\0';
	}

	// Check if it is one of the homes
	if (x == 5 && y > 0 && y < 10) {
		if (y < 5) {
			*index = y - 1;
		} else if (y > 5) {
			*index = 3 - (y - 6);
		}
		return 'h';
	}
	if (y == 5 && x > 0 && x < 10) {
		if (x < 5) {
			*index = y - 1;
		}
		else if (x > 5) {
			*index = 3 - (x - 6);
		}
		return 'h';
	}

	// Figure out the grid position
	if (x == 0) {
		*index = 30 - (y - 4);
	}
	else if (x == 4) {
		*index = y < 5 ? 38 - y : 24 - (y - 6);
	}
	else if (x == 6) {
		*index = y < 5 ? y : y + 8;
	}
	else if (x == 10) {
		*index = y + 4;
	}
	else {
		// If not covered by those columns yet, try rows
		if (y == 0 && x == 5) {
			*index = 39;
		}
		else if (y == 4) {
			*index = x < 5 ? x + 30 : x - 2;
		}
		else if (y == 6) {
			*index = x < 5 ? 28 - x : 14 - (x - 6);
		}
		else if (y == 10 && x == 5) {
			*index = 19;
		}
		else {
			cout << "Missed a figure? x: " << x << " y: " << y << endl;
		}
	}

	return 'g';
}

void putCirclesOnBoard(LudoBoard *lb, vector<Vec3f> circles, LudoColor color) {
	for (int i = 0; i < circles.size(); i++) {
		Vec3f circle = circles[i];
		Vec2i gridPos = getGridPosition(circle);
		int pos = 0;
		char placeCategory = getGridIndex(gridPos, &pos);
		switch (placeCategory)
		{
		case 'h':
			lb->homes[color - 1][pos] = color;
			break;
		case 'g':
			// TODO: Determine correct color
			lb->setAt(pos, color, -1, blue);
		default:
			break;
		}
	}
}

void takePictureMode() {
	while (true)
	{
		Mat frame;
		camera.read(frame);
		imshow("Camera", frame);
		signed char keyPressed = waitKey(30);
		if (keyPressed == 'P' || keyPressed == 'p') {
			imwrite("camerapic.jpg", frame);
			cout << "pic saved!";
		}
		else if (keyPressed == '\r' || keyPressed == '\n') {
			break;
		}
	}
}

void initCamera() {
	if (!camera.isOpened())
	{
		cout << "Unable to access camera" << endl;
		exit(-1);
	}
	cout << "Framerate: " << camera.get(CV_CAP_PROP_FPS) << endl;
	camera.set(CV_CAP_PROP_CONTRAST, 0.5);
}

Mat prevSrc;
int processFrame() {
	// Game logic stuff
	LudoBoard lb = LudoBoard();
	LudoColorLogic colors[4];
	for (int i = 1; i < 5; i++) {
		colors[i - 1] = LudoColorLogic((LudoColor)i);
	}
	colors[blue - 1].isAI = true;
	int current = 0;

	// Do actual image processing yay

	// Get the source
	Mat src, gray, tmp, edges;
	camera.read(src);

	int bigCols = src.cols * 0.3;
	int bigRows = src.rows * 0.3;

	if (!src.data) {
		cout << "Could not read image!";
	}

	cvtColor(src, gray, CV_BGR2GRAY);

	GaussianBlur(gray, tmp, Size(3, 3), 3, 3);
	Canny(tmp, edges, 30, 200, 3);
	vector<vector<Point>> countours;
	findContours(edges, countours, 0, 2);

	vector<Point> outside;
	int rectCountourIndex = 0;
	double maxEuclid = 0;
	for (int i = 0; i < countours.size(); i++) {
		vector<Point> cnt = countours.at(i);
		vector<Point> res;
		approxPolyDP(cnt, res, 0.01 * arcLength(cnt, true), true);
		if (res.size() == 4) {
			double dist1 = euclid(res[0], res[1]);
			double dist2 = euclid(res[0], res[2]);
			double dist3 = euclid(res[0], res[3]);
			double eucl = max(max(dist1, dist2), dist3);
			double const MIN_SIZE = 0.3;
			drawContours(src, countours, i, (0, 255, 0), 1);

			// Only get the biggest rectangle
			if (dist1 > bigCols * MIN_SIZE && dist2 > bigRows * MIN_SIZE && eucl && eucl > maxEuclid) {
				outside = res;
				maxEuclid = eucl;
				rectCountourIndex = i;
			}
		}
	}

	// Get the board only
	Mat boardSrc;
	bool usingOld = false;
	if (outside.size() < 4) {
		cout << "Skipping frame, could not find the table" << endl;
		imshow("Img", src);
		waitKey(1);
		// Use previous frame
		if (prevSrc.cols > 0) {
			boardSrc = prevSrc;
			usingOld = true;
		}
		else {
			return -1;
		}
		return -1;
	}
	else {
		// Find the one that is the most top left - so it stays in same position
		Rect outsideRect = boundingRect(outside);
		int startingI = 0;
		int diff = outsideRect.height + outsideRect.width;
		for (int i = 0; i < outside.size(); i++) {
			int xi = outside[i].x - outsideRect.x;
			int yi = outside[i].y - outsideRect.y;
			int myDiff = xi + yi;
			if (myDiff < diff) {
				startingI = i;
				diff = myDiff;
			}
		}
		vector<Point2f> pts1;
		int count = 0;
		while (count < 4) {
			int i = (count + startingI) % 4;
			pts1.push_back(Point2f(outside[i].x, outside[i].y));
			count++;
		}

		// Transform the image to a nice square
		vector<Point2f> pts2 = { Point2f(0, 0), Point2f(0, 500), Point2f(500, 500), Point2f(500, 0)};
		Mat M = getPerspectiveTransform(pts1, pts2);
		warpPerspective(src, boardSrc, M, Size(500, 500));

		// Check that it has enough circles
		vector<Vec3f> circles;
		cvtColor(boardSrc, gray, CV_BGR2GRAY);
		HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 1, 10, 20, 20, 15, 22);
		if (circles.size() <= 4) {
			return -1;
		}

		prevSrc = boardSrc;
	}

	// Get the circles from the picture
	cvtColor(boardSrc, gray, CV_BGR2GRAY);
	//Canny(gray, edges, 50, 200, 3);

	vector<Vec3f> circles;
	Mat boardBlackCircles = boardSrc.clone();
	//HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 1, par1, par2, par3, 5, 10);
	HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 1, 10, 20, 20, 15, 22);
	for (size_t i = 0; i < circles.size(); i++)
	{
		Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		int radius = cvRound(circles[i][2]);
		circle(boardBlackCircles, center, radius, Scalar(0, 0, 0), 1, 8, 0);
	}

	// Detect the colors of circles
	initColorMasks(boardSrc);


	vector<Vec3f> circles_white, circles_red, circles_yellow, circles_green, circles_blue;
	for (int i = 0; i < circles.size(); i++) {
		Vec3f c = circles[i];
		Scalar s;
		Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		int radius = cvRound(circles[i][2]);
		if (isOfColor(redBBGR, c)) {
			circles_red.push_back(c);
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
		/*else if (isOfColor(&whiteBBGR, c)) {
			s = Scalar(0, 0, 0);
		}*/
		else {
			//cout << "Unknown circle..." << endl;
			circles_white.push_back(c);
			s = Scalar(0, 0, 0);
		}
		putText(boardSrc, to_string(i), center, 0, 0.35, s, 2);
		circle(boardSrc, center, radius, s, 2, 8, 0);
	}

	// Identify which circles mean what
	vector<Vec3f> circles_left_red(circles_red), circles_left_green(circles_green),
		circles_left_blue(circles_blue), circles_left_yellow(circles_yellow), circles_left_white(circles_white),
		outside_red, outside_green, outside_blue, outside_yellow, outside_white;
	seperateOutsideCircles(boardSrc, circles_red, &outside_red, &circles_left_red);
	seperateOutsideCircles(boardSrc, circles_green, &outside_green, &circles_left_green);
	seperateOutsideCircles(boardSrc, circles_blue, &outside_blue, &circles_left_blue);
	seperateOutsideCircles(boardSrc, circles_yellow, &outside_yellow, &circles_left_yellow);

	seperateOutsideCircles(boardSrc, circles_white, &outside_white, &circles_left_white);

	colorCirlces(boardSrc, outside_red);
	colorCirlces(boardSrc, outside_green);
	colorCirlces(boardSrc, outside_blue);
	colorCirlces(boardSrc, outside_yellow);
	colorCirlces(boardSrc, outside_white);


	// Recognize grid positions;
	putCirclesOnBoard(&lb, circles_left_red, red);
	putCirclesOnBoard(&lb, circles_left_blue, blue);
	putCirclesOnBoard(&lb, circles_left_green, green);
	putCirclesOnBoard(&lb, circles_left_yellow, yellow);

	// Set the outside on the logical ludo board (to not mess with code)
	lb.setOut(outside_red.size(), red);
	lb.setOut(outside_green.size(), green);
	lb.setOut(outside_blue.size(), blue);
	lb.setOut(outside_yellow.size(), yellow);

	lb.print();
	
	imshow("Img", edges);
	imshow("Win", boardSrc);

	waitKey(500);

}

	

int main(int argc, char *argv[])
{
	if (cameraMode) {
		camera = VideoCapture(0);
	}
	else {
		camera = VideoCapture("video.mp4");
	}

	initCamera();

	par1 = 10;
	par2 = 20;
	par3 = 20;
	par4 = 20;

	namedWindow("Img", WINDOW_NORMAL);
	resizeWindow("Img", 880, 500);
	namedWindow("Win", WINDOW_NORMAL);
	resizeWindow("Win", 500, 500);

	while (true) {
		processFrame();
	}

	return 0;
}


/*
while (true) {
inRange(boardHSV, Scalar(slider_hue, slider_sat, slider_val), Scalar(slider_up_hue, slider_up_sat, slider_up_val), greenBoard);
createTrackbar("H", "Img", &slider_hue, slider_hue_max, on_trackbar);
createTrackbar("H2", "Img", &slider_up_hue, slider_hue_max, on_trackbar);
createTrackbar("S", "Img", &slider_sat, slider_sat_max, on_trackbar);
createTrackbar("S2", "Img", &slider_up_sat, slider_sat_max, on_trackbar);
createTrackbar("V", "Img", &slider_val, slider_val_max, on_trackbar);
createTrackbar("V2", "Img", &slider_up_val, slider_val_max, on_trackbar);
Mat display;
boardSrc.copyTo(display, greenBoard);
imshow("Img", display);

imshow("src", boardSrc);
waitKey(1);
}*/