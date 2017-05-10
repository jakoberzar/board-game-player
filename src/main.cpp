#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include "ludo.cpp"

using namespace cv;
using namespace std;

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

	while (true)
	{

		camera.read(frame);

		LudoBoard lb = LudoBoard();

		LudoColorLogic colors[4];
		for (int i = 1; i < 5; i++) {
			colors[i - 1] = LudoColorLogic((LudoColor)i);
		}
		colors[1].isAI = true;
		int current = 0;

		// rectangle(frame, Point(20, 20), Point(600, 400), Scalar(0, 255, 0, 0), 3);

		// putText(frame, inputText, Point(100, 150), FONT_HERSHEY_SIMPLEX, 2, Scalar(255, 0, 0, 0), 2);

		imshow("Camera", frame);

		signed char keyPressed = waitKey(30);
		if (keyPressed == 'P' || keyPressed == 'p') {
			imwrite("camerapic.jpg", frame);
			inputText = "pic saved!";
		}
		else if (keyPressed == '\r' || keyPressed == '\n') {
			break;
		}
	}

	return 0;
}
