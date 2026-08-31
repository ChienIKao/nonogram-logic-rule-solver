#ifndef BOARD_H
#define BOARD_H

#include "cdef.h"

class Board {
	public:
		uint64_t data[50];
		uint64_t oldData[50];
		uint16_t size;

		int probNum = -1;

		Board();

		bool operator<(const Board& rhs) const {
			for (int i = 0; i < 25; ++i)
				if (data[i] < rhs.data[i]) return true;
			return false;
		}
		bool operator==(const Board& rhs) const {
			for (int i = 0; i < 25; ++i)
				if (data[i] != rhs.data[i]) return false;
			return true;
		}
};

void writeBack(Board& board, int lineNum, int line);

void merge(Board& a, Board& b);

void setBit(Board& board, int x, int y, int val);
int getBit(const Board& board, int x, int y);

void setLine(Board& board, int line, uint64_t val);
uint64_t getLine(Board& board, int line);

int getSize(Board& board);

void printBoard(Board& board, char*, int);
int debugBoard(Board& board);
int compareBoard(Board&);
int checkBoard(Board& b);

int hasNewPuzzle(Board&);

#endif
