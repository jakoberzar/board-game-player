#include <stdexcept>
#include <iostream>
#include <vector>
#include <stdio.h>
#include "ludo.h"

#define FIELD_LEN 44

using namespace std;

// Initializes the board (like a constructor)
void LudoBoard::init() {
	// Inititalize the field (should be a constructor in the future...
	for (int i = 0; i < 4; i++) {
		out[i] = 4;
		for (int j = 0; j < 10; j++) {
			tracks[i][j] = none;
		}
		for (int j = 0; j < 4; j++) {
			homes[i][j] = none;
		}
	}
}

// Gets the pointer to the element on board at given index
LudoColor* LudoBoard::getPtr(int index, LudoColor myColor) {
	if (index < 0) {
		throw invalid_argument("Index under 0, out of bounds!");
		return NULL;
	}

	int count = 0;
	int currentColor = myColor;
	for (int i = 1; i < 5; i++) {
		if (index - count < 10) {
			return &(tracks[currentColor - 1][index - count]);
		}
		count += 10;
		currentColor = (currentColor + 1) % 5;
		if (currentColor == none) {
			currentColor++;
		}
	}

	if (index - count < 4) {
		return &(homes[currentColor - 1][index - count]);
	}

}

// Gets the color at given index
LudoColor LudoBoard::getAt(int index, LudoColor myColor) {
	LudoColor *color = getPtr(index, myColor);
	if (color != NULL) {
		return *color;
	}
}

// Sets the color at given index
bool LudoBoard::setAt(int index, LudoColor myColor, int previous, LudoColor boardColor) {
	// Check if everything makes sense
	if (boardColor == none) boardColor = myColor;
	int currentThere = getAt(index, boardColor);

	if (previous != -1) {
		int previousPos = getAt(previous, boardColor);
		if (previousPos != myColor) {
			throw std::invalid_argument("Previous position does not match the color!");
			return false;
		}
		if (previous > index || index - previous > 6) {
			throw std::invalid_argument("Invalid move!");
			return false;
		}
	}

	if (currentThere > none) {
		cout << "Rekt " << currentThere << "!" << endl;
		removeFigure(index, myColor);
	}

	// Set the color, code similar to getAt
	LudoColor *color = getPtr(index, boardColor);
	if (color != NULL) {
		*color = myColor;
	}

	if (previous != -1) {
		LudoColor *colorPrev = getPtr(previous, boardColor);
		if (color != NULL) {
			*colorPrev = none;
		}
	}
	else {
		out[myColor] -= 1;
	}

	return true;
 }

// Puts a figure of given color on the field
bool LudoBoard::putOnField(LudoColor myColor) {
	// First, check if the player has any figures left outside...
	int colorNum = myColor - 1;
	if (out[colorNum] <= 0) {
		throw std::invalid_argument("No more figures of this color available!");
		return false;
	}

	LudoColor currentThere = getAt(0, myColor);
	if (currentThere) {
		cout << "Rekt " << currentThere << "!" << endl;
		removeFigure(0, myColor);
	}

	// Set the color, code similar to getAt
	LudoColor *color = getPtr(0, myColor);
	if (color != NULL) {
		*color = myColor;
	}

	out[colorNum] -= 1;
	return true;
}

// Removes a figure at given index from the field
bool LudoBoard::removeFigure(int index, LudoColor myColor) {
	int currentThere = getAt(index, myColor);
	if (currentThere == NULL || currentThere == none) {
		throw new std::invalid_argument("No figure currently there");
		return false;
	}
	else {
		int currentColorNum = currentThere - 1;
		out[currentColorNum] += 1;
		currentThere = none;
		return true;
	}
}

// Tells if the given index is home
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

// Setter for out array
void LudoBoard::setOut(int count, LudoColor myColor) {
	out[myColor - 1] = count;
}

// Checks if the suggested move is legit
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

// Checks that the board is in a legit state
bool LudoBoard::boardLegit() {
	for (int i = 1; i < 5; i++) {
		int count = out[i - 1]; // Add my figures that are outside
		LudoColor myColor = (LudoColor)i;
		for (int j = 0; j < FIELD_LEN; j++) {
			if (getAt(j, myColor) == myColor) {
				count++;
			}
		}
		if (count != 4) {
			return false;
		}
	}
	return true;
}

// Prints the current board
void LudoBoard::print() {
	// Get the fields for out first
	LudoColor topRight = blue;

	char outFields[4][4];
	int i = red - 1; // current color - index
	for (int m = 0; m < 4; m++) {
		// Each color
		for (int j = 0; j < 4; j++) {
			if (out[i] > j) {
				outFields[m][j] = colorChar((LudoColor)(i + 1));
			}
			else {
				outFields[m][j] = colorChar((LudoColor)none);
			}
		}
		i = (i + 1) % 4;
	}

	char homeFields[4][4];
	i = blue - 1; // current color - index
	for (int m = 0; m < 4; m++) {
		// Each color
		for (int j = 0; j < 4; j++) {
			homeFields[m][j] = colorChar(homes[i][j]);
		}
		i = (i + 1) % 4;
	}

	char field[40];
	for (int i = 0; i < 40; i++) {
		field[i] = colorChar(getAt(i, topRight));
	}

	// Order: Y G
	//        B R
	printf("[%c][%c]      ---------      [%c][%c]\n",
		outFields[0][0], outFields[0][1],
		outFields[1][1], outFields[1][0]);
	printf("[%c][%c]      |%c||%c||%c|      [%c][%c]\n",
		outFields[0][2], outFields[0][3],
		field[38], field[39], field[0],
		outFields[1][3], outFields[1][2]);
	printf("            |%c|[%c]|%c|\n", field[37], homeFields[0][0], field[1]);
	printf("            |%c|[%c]|%c|\n", field[36], homeFields[0][1], field[2]);
	printf("------------|%c|[%c]|%c|------------\n", field[35], homeFields[0][2], field[3]);
	printf("|%c||%c||%c||%c||%c|[%c]|%c||%c||%c||%c||%c|\n",
		field[30], field[31], field[32], field[33], field[34],
		homeFields[0][3],
		field[4], field[5], field[6], field[7], field[8]);
	printf("|%c|[%c][%c][%c][%c]   [%c][%c][%c][%c]|%c|\n",
		field[29],
		homeFields[3][0], homeFields[3][1], homeFields[3][2], homeFields[3][3],
		homeFields[1][3], homeFields[1][2], homeFields[1][1], homeFields[1][0],
		field[9]);
	printf("|%c||%c||%c||%c||%c|[%c]|%c||%c||%c||%c||%c|\n",
		field[28], field[27], field[26], field[25], field[24],
		homeFields[2][3],
		field[14], field[13], field[12], field[11], field[10]);
	printf("------------|%c|[%c]|%c|------------\n", field[23], homeFields[2][2], field[15]);
	printf("            |%c|[%c]|%c|\n", field[22], homeFields[2][1], field[16]);
	printf("            |%c|[%c]|%c|\n", field[21], homeFields[2][0], field[17]);
	printf("[%c][%c]      |%c||%c||%c|      [%c][%c]\n",
		outFields[3][2], outFields[3][3],
		field[20], field[19], field[18],
		outFields[2][3], outFields[2][2]);
	printf("[%c][%c]      ---------      [%c][%c]\n",
		outFields[3][0], outFields[3][1],
		outFields[2][1], outFields[2][0]);
}

// Returns the char representation of given color
char LudoBoard::colorChar(LudoColor c) {
	switch (c) {
		case none:
			return ' ';
		case red:
			return 'R';
		case green:
			return 'G';
		case blue:
			return 'B';
		case yellow:
			return 'Y';
		default:
			return '-';
	}
}

// Compares a board with a different one.
// Returns null if same, o if outside, h if home, g if grid
string LudoBoard::diff(LudoBoard *other, int *index1, int *index2, int *index3) {
	// Go through every player
	bool illegal = false;
	string ret = "";
	for (int c = 0; c < 4; c++) {
		LudoColor myColor = (LudoColor)(c + 1);
		// Check the number of figures outside
		if (out[c] != other->out[c]) {
			*index1 = c;
			*index2 = other->out[c];
			*index3 = out[c];
			return "outside";
		}
		else {
			// Look for changes in field
			int figuresHere = out[c];
			int figuresThere = other->out[c];
			int oldIndex = -1;
			for (int i = 0; i < FIELD_LEN; i++) {
				if (getAt(i, myColor) == myColor && other->getAt(i, myColor) != myColor) {
					if (other->getAt(i, myColor) == none) {
						// The figure has probably just been moved...
						oldIndex = i;
						*index3 = i;
					}
					else {
						// Some figure crashed mine but the number of my figures outside didn't change???
						cout << "Some figure crashed " << colorChar(myColor) << " at " << i << " but its out count didnt change?";
						return "illegal";
					}
					figuresHere++;
				}
				else if (getAt(i, myColor) == none && other->getAt(i, myColor) == myColor) {
					// The figure was moved to this field
					*index2 = i;
					*index1 = c;
					return "grid";
				}
				
			}
					
		}

	}

	return "\0";	
}


LudoColorLogic::LudoColorLogic(LudoColor color) {
	if (color == none) {
		throw invalid_argument("Wrong color for a ludo color logic!");
	}
	myColor = color;
}

vector<int> LudoColorLogic::getMyFigures(LudoColor intColor) {
	vector<int> vec(4);
	for (int i = 0; i < FIELD_LEN; i++) {
		LudoColor color = board->getAt(i, intColor);
		if (color == myColor) {
			vec.push_back(i);
		}
	}
	if (vec.size() != 4 - board->out[intColor - 1]) {
		cout << "Problem with the number of figures out and on board for " << intColor << endl;
	}
	return vec;
}

LudoAI::LudoAI(LudoColorLogic *lgl) {
	boardLogic = lgl;
}

int LudoAI::getNewPos(int diceThrow) {
	vector<int> figures = boardLogic->getMyFigures(boardLogic->myColor);
	vector<int>::iterator it;
	int figureToMove = -1;
	for (it = figures.begin(); it < figures.end(); it++) {
		int figure = *it;
		if (boardLogic->board->legitMove(figure + diceThrow, boardLogic->myColor, figure)) {
			figureToMove = figure;
		}
	}
	int newPos = figureToMove == -1 ? diceThrow : figureToMove + diceThrow;
	boardLogic->board->setAt(newPos, boardLogic->myColor, figureToMove);
	return newPos;
}