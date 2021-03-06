#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>
#include <random>
#include "ludo.h"

#define CAMERA_MODE false

using namespace cv;
using namespace std;

// ----------------------
// Function headers
// ----------------------
void initCamera();
int processFrame();
// Detection
int circleOutside(Mat board, Vec3f circle);
int circleHome(Mat board, Vec3f circle);
bool extractBoard(Mat *wholeBoard, Mat *smallBoard, vector<Point> outside);
vector<Point> findBoard(Mat *grayBoard);
Vec2i getGridPosition(Vec3f circle);
char getGridIndex(Vec2i gridPosition, int *index);
void initColorMasks(Mat boardCircles);
bool isOfColor(Mat board, Vec3f circle);
void seperateOutsideCircles(Mat board, vector<Vec3f> all_circles, vector<Vec3f> *outside_circles, vector<Vec3f> *remaining);
// Display
void colorCirlces(Mat board, vector<Vec3f> circles, int thickness = -1);
// Logical
void putCirclesOnBoard(LudoBoard *lb, vector<Vec3f> circles, LudoColor color);
void processDifferences();
// Helper
Vec3b averageCirclePixel(Mat src, Point center, int radius);
int euclid(Point a, Point b);
void on_trackbar(int, void*);

// ----------------------
// Global variables
// ----------------------
// Important stuff
VideoCapture camera;  // ID of the (back) camera
LudoBoard *lb, *oldBoard;
LudoColor myPlayer = red;
// Color masks
Mat boardHSV, whiteBoard, redBoard, redBoard1, redBoard2, greenBoard, yellowBoard, blueBoard;
Mat whiteBBGR, redBBGR, greenBBGR, yellowBBGR, blueBBGR;
// AI stuff
bool waitingForDiceRoll = false;
bool waitingForMyPlayerMove = false;
int myPlayerMove = 0;
// Display stuff
bool correctFrameShown = false;

// For sliders
int par1, par2, par3, par4;

// For improving color (hue) filters
//const int slider_hue_max = 180;
//const int slider_sat_max = 255;
//const int slider_val_max = 255;
//int slider_hue = 100;
//int slider_sat = 150;
//int slider_val = 0;
//int slider_up_hue = 135;
//int slider_up_sat = 225;
//int slider_up_val = 255;

// ----------------------
// Methods
// ----------------------
int main(int argc, char *argv[])
{
	if (CAMERA_MODE) {
		camera = VideoCapture(0);
	}
	else {
		camera = VideoCapture("video2.mp4");
	}

	initCamera();

	par1 = 10;
	par2 = 20;
	par3 = 20;
	par4 = 20;

	if (!CAMERA_MODE) {
		namedWindow("Img", WINDOW_NORMAL);
		resizeWindow("Img", 880, 500);
		namedWindow("Win", WINDOW_NORMAL);
		resizeWindow("Win", 500, 500);
	}

	oldBoard = new LudoBoard();
	oldBoard->init();
	while (true) {
		processFrame();
	}

	return 0;
}

// Simple helper function for calculating distance between two points
int euclid(Point a, Point b) {
	return sqrt(pow((a.x - b.x), 2) + pow((a.y - b.y), 2));
}

// Handler for opencv trackbar
void on_trackbar(int, void*) { }

// Calculates the values of an average value in given circle area
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

// Tells whether the circle has content on given board.
// The board should only show elements of a certain color.
bool isOfColor(Mat board, Vec3f circle) {
	Point2i p1(cvRound(circle[0] - circle[2]), cvRound(circle[1] - circle[2]));
	Point2i p2(cvFloor(circle[0] + circle[2]), cvFloor(circle[1] + circle[2]));
	Rect r = Rect(p1, p2);
	Mat mask, src, gray, tmp, edges;
	mask = Mat(board.rows, board.cols, board.type());
	mask.setTo(cv::Scalar(0, 0, 0));

	int index;
	Vec2i grid = getGridPosition(circle);
	char pos = getGridIndex(grid, &index);
	int radius = pos != 'h' ? cvRound(circle[2]) - 6: 5;
	//int radius = 5;
	cv::circle(mask, Point(cvRound(circle[0]), cvRound(circle[1])), radius, Scalar(255, 255, 255), -1, 8, 0);

	(board).copyTo(src, mask);
	if (r.x + r.width > 500) r.width = 500 - r.x;
	if (r.y + r.height > 500) r.height = 500 - r.y;
	if (r.y < 0) r.y = 0;
	if (r.x < 0) r.x = 0;
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

// Returns the index of home. Deprecated.
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

// Color all the circles
void colorCirlces(Mat board, vector<Vec3f> circles, int thickness) {
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
		else {
			s = Scalar(0, 0, 0);
		}
		Point center(cvRound(c[0]), cvRound(c[1]));
		int radius = cvRound(c[2]);
		circle(board, center, radius, s, thickness, 8, 0);
	}
}

// Initialize the color mask matrices and apply them
void initColorMasks(Mat boardCircles) {
	cvtColor(boardCircles, boardHSV, COLOR_BGR2HSV);
	inRange(boardHSV, Scalar(75, 0, 135), Scalar(130, 115, 255), whiteBoard);
	inRange(boardHSV, Scalar(0, 70, 100), Scalar(10, 255, 255), redBoard1);
	inRange(boardHSV, Scalar(135, 70, 100), Scalar(180, 255, 255), redBoard2);
	bitwise_or(redBoard1, redBoard2, redBoard);
	inRange(boardHSV, Scalar(20, 90, 100), Scalar(35, 255, 255), yellowBoard);
	inRange(boardHSV, Scalar(40, 60, 00), Scalar(90, 255, 255), greenBoard);
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

// Seperate outside circles from the others
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

// Get the position of given element on the 11x11 grid
Vec2i getGridPosition(Vec3f circle) {
	double singleWidth = 500.0 / 11.0;
	return Vec2i(circle[0] / singleWidth, circle[1] / singleWidth);
}

// Returns g if it is on grid, h if on home, null if not. Stores the field location in index parameter
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
			*index = x - 1;
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

// Puts the circles on the logical board
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

// Enables picture mode, for taking pictures (for debugging help)
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

// Inits the camera
void initCamera() {
	if (!camera.isOpened())
	{
		cout << "Unable to access camera" << endl;
		exit(-1);
	}
	cout << "Framerate: " << camera.get(CV_CAP_PROP_FPS) << endl;
	camera.set(CV_CAP_PROP_CONTRAST, 0.5);
}

// Finds the game board on the screen
vector<Point> findBoard(Mat *grayBoard) {
	vector<vector<Point>> countours;
	vector<Point> outside;
	Mat edges = *grayBoard;
	double const MIN_SIZE = 0.3;
	int bigCols = edges.cols * MIN_SIZE;
	int bigRows = edges.rows * MIN_SIZE;
	findContours(edges, countours, 0, 2);
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
			drawContours(edges, countours, i, (0, 255, 0), 1);

			// Only get the biggest rectangle
			if (dist1 > bigCols && dist2 > bigRows && eucl && eucl > maxEuclid) {
				outside = res;
				maxEuclid = eucl;
			}
		}
	}
	return outside;
}

// Extracts the board from the whole board. Returns if given board is okay
bool extractBoard(Mat *wholeBoard, Mat *smallBoard, vector<Point> outside) {
	Mat src = *wholeBoard;
	Mat boardSrc, gray;

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
	vector<Point2f> pts2 = { Point2f(0, 0), Point2f(0, 500), Point2f(500, 500), Point2f(500, 0) };
	Mat M = getPerspectiveTransform(pts1, pts2);
	warpPerspective(src, boardSrc, M, Size(500, 500));

	// Check that it has enough circles
	vector<Vec3f> circles;
	cvtColor(boardSrc, gray, CV_BGR2GRAY);
	HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 1, 30, 20, 30, 13, 30);
	//colorCirlces(src, circles);
	*smallBoard = boardSrc;

	if (circles.size() <= 4) {
		return false;
	}
	else {
		return true;
	}

}

// Processes differences between previous frame board and current frame board
void processDifferences() {
	int index1, index2, index3;
	string diff = oldBoard->diff(lb, &index1, &index2, &index3);

	if (waitingForDiceRoll || waitingForMyPlayerMove && diff != "\0") {
		cout << endl << "You just skipped me!!! :o" << endl;
		cout << "Please move my player for " << myPlayerMove << " spaces!" << endl;
	}

	if (waitingForMyPlayerMove && diff != "\0") {
		if (index1 + 1 == myPlayer) {
			cout << "Thank you for moving my player for " << index2 - index3 << " spaces!" << endl;
			waitingForMyPlayerMove = false;
		}
	}

	if (diff != "\0") {
		char colorChanged = lb->colorChar((LudoColor)(index1 + 1));
		cout << endl;
		if (diff == "outside") {
			cout << "Player " << colorChanged << "'s outside count changed ";
			cout << "from " << index3 << " to " << index2;
		}
		else if (diff == "grid") {
			cout << "Player " << colorChanged << "'s figure moved ";
			cout << "from " << index3 << " to " << index2;
		}
		else if (diff == "illegal") {
			cout << "Something illegal happened!";
		}
		cout << endl;

		lb->print();

		oldBoard = lb;

		if (index1 + 2 == myPlayer || index1 == 4 && myPlayer == 1) {
			waitingForDiceRoll = true;
			cout << "Please throw a dice for me :)?" << endl;
		}
	}
	else {
		cout << ":";
	}
}

// Process a frame
int processFrame() {
	Mat src, gray, tmp, edges;

	camera.read(src);
	if (!src.data) {
		cout << "Could not read image!";
		exit(0);
	}

	cvtColor(src, gray, CV_BGR2GRAY);
	GaussianBlur(gray, tmp, Size(3, 3), 3, 3);
	Canny(tmp, edges, 30, 200, 3);

	vector<Point> outside = findBoard(&edges);

	// Get the board only
	Mat boardSrc;
	bool usingOld = false;
	if (outside.size() < 4) {
		cout << ".";
		if (CAMERA_MODE) {
			imshow("Img", edges);
		}
		else {
			imshow("Img", src);
		}

		waitKey(1);
		return -1;
	}
	else {
		if (!extractBoard(&src, &boardSrc, outside)) {
			return -1;
		}
	}

	// Get the circles from the picture
	cvtColor(boardSrc, gray, CV_BGR2GRAY);

	vector<Vec3f> circles;
	Mat boardBlackCircles = boardSrc.clone();
	HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 1, 30, 20, 30, 13, 30);
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
		else {
			circles_white.push_back(c);
			s = Scalar(0, 0, 0);
		}
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

	// Game logic stuff
	lb = new LudoBoard();
	lb->init();
	LudoColorLogic colors[4];
	for (int i = 1; i < 5; i++) {
		colors[i - 1] = LudoColorLogic((LudoColor)i);
	}
	colors[red - 1].isAI = true;
	int current = 0;

	// Recognize grid positions;
	putCirclesOnBoard(lb, circles_left_red, red);
	putCirclesOnBoard(lb, circles_left_blue, blue);
	putCirclesOnBoard(lb, circles_left_green, green);
	putCirclesOnBoard(lb, circles_left_yellow, yellow);

	// Set the outside on the logical ludo board (to not mess with code)
	lb->setOut(outside_red.size(), red);
	lb->setOut(outside_green.size(), green);
	lb->setOut(outside_blue.size(), blue);
	lb->setOut(outside_yellow.size(), yellow);

	// Determine whether the board actually makes sense (in case some circle was not detected)
	bool legit = lb->boardLegit();

	if (!legit && !correctFrameShown) {
		imshow("Win", boardSrc);
		waitKey(1);
	}

	if (legit) {
		// If waiting for dice, do stuff here
		if (waitingForDiceRoll) {
			Rect area = Rect(225, 225, 50, 50);
			Mat diceMat = gray(area);
			Canny(diceMat, edges, 30, 200, 3);
			vector<vector<Point>> countours;
			findContours(edges, countours, 0, 2);

			vector<Point> outside;
			for (int i = 0; i < countours.size(); i++) {
				vector<Point> cnt = countours.at(i);
				vector<Point> res;
				approxPolyDP(cnt, res, 0.01 * arcLength(cnt, true), true);
				if (res.size() == 4) {
					Rect bound = boundingRect(res);
					HoughCircles(gray(bound), circles, CV_HOUGH_GRADIENT, 1, 1, 20, 30, 1, 3);
					if (circles.size() > 0) {
						cout << "Looks like I threw " << circles.size() << "!" << endl;
						cout << "Move my first player!" << endl;
						waitingForMyPlayerMove = true;
						myPlayerMove = circles.size();
						waitingForDiceRoll = false;
						break;
					}
				}
			}
			if (!waitingForMyPlayerMove) { cout << "?"; }

			// For now, try throwing a dice yourself
			random_device rd;
			mt19937 rng(rd());
			uniform_int_distribution<int> uni(1, 6);
			int random_integer = uni(rng);
			int together_move = random_integer;
			while (random_integer == 6) {
				cout << "Woah, I got a 6!" << endl;
				random_integer = uni(rng);
				together_move += random_integer;
			}
			myPlayerMove = together_move;
			waitingForDiceRoll = false;
			waitingForMyPlayerMove = true;
			cout << "Please move my player for " << myPlayerMove << " spaces!" << endl;
		}

		// Show stuff
		imshow("Img", src);
		imshow("Win", boardSrc);

		processDifferences();

		correctFrameShown = true;
	}
	waitKey(100);
}

// Code for tweaking the hue parameters.
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