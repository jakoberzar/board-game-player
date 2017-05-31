#include <vector>

using namespace std;

enum LudoColor {
	none,
	yellow,
	green,
    red,
    blue,
};

class LudoBoard {
public:
	// The whities
	static LudoColor tracks[4][10];
	// The homes
	static LudoColor homes[4][4];
	// Out of playing field
	static int out[4];

	LudoBoard() {

	}; // Disallow creation
	~LudoBoard() {

	}

	static void init(); // Should be non static in future

	static LudoColor getAt(int index, LudoColor myColor);

	static bool setAt(int index, LudoColor myColor, int previous);

	static LudoColor* getPtr(int index, LudoColor myColor);

	static bool putOnField(LudoColor color);

	static bool removeFigure(int index, LudoColor myColor);

	static bool isHome(int index, LudoColor myColor);

	static bool legitMove(int index, LudoColor myColor, int previous);

	static void print();

	static char colorChar(LudoColor c);

private:
};

// A wrapper class for the color, so we dont interact with the board directly
class LudoColorLogic {
public:
    LudoColor myColor;
	bool isAI;

	LudoColorLogic() {};
	LudoColorLogic(LudoColor color);

	vector<int> getMyFigures(LudoColor intColor);
};

class LudoAI {
public:
	LudoColorLogic boardLogic;

	LudoAI(LudoColorLogic lgl);

	int getNewPos(int diceThrow);
};