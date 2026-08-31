#include "board.h"

#include <cstdio>

using namespace std;

Board::Board() {
	for (int i = 0; i < 50; ++i) {
		data[i] = FILL;
		oldData[i] = 0;
	}
	size = 0;
}

void merge(Board& a, Board& b) {
	for (int i = 0; i < 50; ++i) a.data[i] &= b.data[i];
}

int getSize(Board& board) {
	board.size = 25 * 50;
	for (int i = 0; i < 25; ++i)
		board.size -= __builtin_popcountll(board.data[i]);

	return board.size;
}

// only check if has illegal bit
int checkBoard(Board& b) {
	for (int i = 0; i < 50; ++i) {
		uint64_t test = (b.data[i] & 0x5555555555555555LL) |
		                (b.data[i] >> 1 & 0x5555555555555555LL);
		if (test != 0x1555555555555LL) return 1;
	}
	return 0;
}

void setBit(Board& board, int x, int y, int val) {
	__SET(board.data[x], y, val);
	__SET(board.data[y + 25], x, val);
}

int getBit(const Board& board, int x, int y) { return __GET(board.data[x], y); }

void setLine(Board& board, int line, uint64_t val) {
	if (line < 25) {
		board.data[line] = val;
		for (int k = 0; k < 25; ++k) {
			__SE(board.data[k + 25], line, __GET(val, k));
			__builtin_prefetch(&board.data[k + 26], 1);
		}
	} else {
		board.data[line] = val;
		for (int k = 0; k < 25; ++k) {
			__SE(board.data[k], (line - 25), __GET(val, k));
			__builtin_prefetch(&board.data[k + 1], 1);
		}
	}
}

uint64_t getLine(Board& board, int x) { return board.data[x]; }

void printBoard(Board& board, char* fileName, int probN) {
	FILE* out = fopen(fileName, "a+");
	fprintf(out, "$%d\n", probN);
	for (int i = 0; i < 25; ++i) {
		for (int j = 0; j < 25; ++j) {
			uint64_t val = __GET(board.data[j], i);
			if (BIT_ZERO == val)
				fprintf(out, "0");
			else if (BIT_ONE == val)
				fprintf(out, "1");
			else
				fprintf(out, "?");

			if (j == 24)
				fprintf(out, "\n");
			else
				fprintf(out, "\t");
		}
	}

	if (debugBoard(board) == 625) {
		fprintf(out, "Solved\n");
	} else {
		fprintf(out, "Unsolved\n");
	}
	fclose(out);
}

int debugBoard(Board& board) {
	int count = 0;
	for (int i = 0; i < 25; ++i) {
		for (int j = 0; j < 25; ++j) {
			uint64_t val = __GET(board.data[i], j);
			if (BIT_ZERO == val) {
				// printf( "O" );
				count++;
			} else if (BIT_ONE == val) {
				// printf( "@" );
				count++;
			}
		}
	}
	return count;
}

int compareBoard(Board& board) {
	for (int i = 0; i < 50; i++) {
		// printf("oldData = %llu\nnewData = %llu\n", board.oldData[i],
		// board.data[i] );
		if (board.oldData[i] != board.data[i]) return INCOMP;
	}
	return SOLVED;
}

int hasNewPuzzle(Board& board) {
	int hasNew = 0;
	for (int i = 0; i < 25; i++) {
		for (int j = 0; j < 25; j++) {
			if (__GET(board.data[i], j) != BIT_UNKNOWN) {
				hasNew++;
			}
		}
	}
	return hasNew;
}
