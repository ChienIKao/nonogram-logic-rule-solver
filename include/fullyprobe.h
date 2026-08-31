#ifndef FULLY_PROBE_H
#define FULLY_PROBE_H

#include "board.h"
#include "cdef.h"
#include "logicSolve.h"
#include "set.h"

struct TempC {       // (2) 用於存儲線索的臨時結構，包含 Rc 和 Cc
		int i = 0;   // 行
		int j = 0;   // 列
		int n = 0;   // i*25+j
		int cal = 0; // sum、max、min
		int Rc = 0;
		int Cc = 0;
};

class FullyProbe {
	public:
		Board gp[25][25][2];
		Board max_g0, max_g1, u_board;

		int method;
		int setting;
		int PropagateCount = 0;
		int probGtimes = 0;
		int AfterProbePixels = 0;

		myset P;
		myset oldP;

		// test
		Board mainBoard;
		double eigen[25][25];
		void clear() { MEMSET_ZERO(eigen); }
};

double choose(int method, double mp1, double mp0);
int fp2(FullyProbe&, LogicSolve&, Board&);
int probe(FullyProbe&, LogicSolve&, Board&, int, int);
int probeG(FullyProbe&, LogicSolve&, int, int, uint64_t);
void setBestPixel(FullyProbe&, Board&);

#endif
