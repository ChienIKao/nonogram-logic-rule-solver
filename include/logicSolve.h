#ifndef LOGICSOLVE_H
#define LOGICSOLVE_H

#include "board.h"
#include "cdef.h"

struct Record {
		int R;
		int C;
		int cal;
};

class LogicSolve {
	public:
		Clue clue[50];
		Pixel leftMost[50][14], rightMost[50][14];
		Board solvedBoard;
		Record r[625];

		bool changedLine[50];

		LogicSolve();

		void load(int* data);
		void init_change();
};

int Propagate(LogicSolve& ls, Board& board);
int Logic_solve(LogicSolve& ls, Board& board);
int RLmost_init(LogicSolve& ls, Board& board);
int RLmost(LogicSolve& ls, int lineNum, const uint64_t& line);
int Update_leftmost(LogicSolve& ls, int lineNum, const uint64_t& line,
                    int start, int currClue);
int Update_rightmost(LogicSolve& ls, int lineNum, const uint64_t& line,
                     int start, int currClue);
int Logic_rule(LogicSolve& ls, Board& board);

#endif
