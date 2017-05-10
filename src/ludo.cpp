#include <stdexcept>
#include <iostream>
#include <vector>

#define FIELD_LEN 44

using namespace std;

enum LudoColor {
	none,
    red,
    green,
    blue,
    yellow
};

class LudoBoard {
public:
    // The whities
    static LudoColor tracks[4][10];
    // The homes
    static LudoColor homes[4][4];

	LudoBoard() {}; // Disallow creation

	static LudoColor getAt(int index, LudoColor myColor);

	static bool setAt(int index, LudoColor myColor, int previous);

	static LudoColor* getPtr(int index, LudoColor myColor);

	static bool isHome(int index, LudoColor myColor);

	static bool legitMove(int index, LudoColor myColor, int previous);

private:
};
LudoColor* LudoBoard::getPtr(int index, LudoColor myColor) {
	if (index < 0) {
		throw invalid_argument("Index under 0, out of bounds!");
		return NULL;
	}

	int count = 0;
	int currentColor = myColor;
	for (int i = 1; i < 5; i++) {
		if (index - count < 10) {
			return &(tracks[i - 1][index - count]);
		}
		count += 10;
		currentColor = (currentColor + 1) % 4;
		if (currentColor == none) {
			currentColor++;
		}
	}

	if (index - count < 4) {
		return &(homes[currentColor - 1][index - count]);
	}

}

LudoColor LudoBoard::getAt(int index, LudoColor myColor) {
	LudoColor *color = getPtr(index, myColor);
	if (color != NULL) {
		return *color;
	}
}

bool LudoBoard::setAt(int index, LudoColor myColor, int previous) {
	// Check if everything makes sense
	int currentThere = getAt(index, myColor);
	int previousPos = getAt(previous, myColor);
	if (previousPos != myColor) {
		throw std::invalid_argument("Previous position does not match the color!");
		return false;
	}
	if (previous > index || index - previous > 6) {
		throw std::invalid_argument("Invalid move!");
		return false;
	}
	if (currentThere) {
		cout << "Rekt " << currentThere << "!" << endl;
	}

	// Set the color, code similar to getAt
	LudoColor *color = getPtr(index, myColor);
	if (color != NULL) {
		*color = myColor;
	}

	return true;
 }

bool LudoBoard::isHome(int index, LudoColor myColor) {
	if (myColor == none) {
		throw invalid_argument("Color is none!");
		return false;
	}
	bool setTillThere = true;
	for (int i = 3; i > (index - 40); i--) {
		LudoColor c = homes[myColor - 1][i];
		if (c == none) {
			setTillThere = false;
		}
	}
	return setTillThere;
}

bool LudoBoard::legitMove(int index, LudoColor myColor, int previous) {
	// Check if everything makes sense
	int currentThere = getAt(index, myColor);
	int previousPos = getAt(previous, myColor);
	if (previousPos != myColor && previous != -1) {
		throw std::invalid_argument("Previous position does not match the color!");
		return false;
	}
	if (previous > index || index - previous > 6 || isHome(previous, myColor)) {
		return false;
	}

	return true;
}


// A wrapper class for the color, so we dont interact with the board directly
class LudoColorLogic {
public:
    LudoColor myColor;
	bool isAI;

	LudoColorLogic() {};
	LudoColorLogic(LudoColor color);

	vector<int> getMyFigures(LudoColor intColor);
};

LudoColorLogic::LudoColorLogic(LudoColor color) {
	if (color == none) {
		throw invalid_argument("Wrong color for a ludo color logic!");
	}
	myColor = color;
}

vector<int> LudoColorLogic::getMyFigures(LudoColor intColor) {
	vector<int> vec(4);
	for (int i = 0; i < FIELD_LEN; i++) {
		LudoColor color = LudoBoard::getAt(i, intColor);
		if (color == myColor) {
			vec.push_back(i);
		}
	}
	return vec;
}

class LudoAI {
public:
	LudoColorLogic boardLogic;

	LudoAI(LudoColorLogic lgl);

	int getNewPos(int diceThrow);
};

LudoAI::LudoAI(LudoColorLogic lgl) {
	boardLogic = lgl;
}

int LudoAI::getNewPos(int diceThrow) {
	vector<int> figures = boardLogic.getMyFigures(boardLogic.myColor);
	vector<int>::iterator it;
	int figureToMove = -1;
	for (it = figures.begin(); it < figures.end(); it++) {
		int figure = *it;
		if (LudoBoard::legitMove(figure + diceThrow, boardLogic.myColor, figure)) {
			figureToMove = figure;
		}
	}
	int newPos = figureToMove == -1 ? diceThrow : figureToMove + diceThrow;
	LudoBoard::setAt(newPos, boardLogic.myColor, figureToMove);
}