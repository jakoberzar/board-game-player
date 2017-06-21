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
	LudoColor tracks[4][10];
	// The homes
	LudoColor homes[4][4];
	// Out of playing field
	int out[4];

	LudoBoard() {

	};
	~LudoBoard() {

	}

	void init();

	LudoColor getAt(int index, LudoColor myColor);

	bool setAt(int index, LudoColor myColor, int previous, LudoColor boardColor = none);

	LudoColor* getPtr(int index, LudoColor myColor);

	bool putOnField(LudoColor color);

	bool removeFigure(int index, LudoColor myColor);

	bool isHome(int index, LudoColor myColor);

	void setOut(int count, LudoColor myColor);

	bool legitMove(int index, LudoColor myColor, int previous);

	bool boardLegit();

	void print();

	static char colorChar(LudoColor c);

	string diff(LudoBoard *other, int *index, int *index2, int *index3);

private:
};

// A wrapper class for the color, so we dont interact with the board directly
class LudoColorLogic {
public:
    LudoColor myColor;
	LudoBoard *board;
	bool isAI;

	LudoColorLogic() {};
	LudoColorLogic(LudoColor color);

	vector<int> getMyFigures(LudoColor intColor);
};

class LudoAI {
public:
	LudoColorLogic *boardLogic;

	LudoAI(LudoColorLogic *lgl);

	int getNewPos(int diceThrow);
};