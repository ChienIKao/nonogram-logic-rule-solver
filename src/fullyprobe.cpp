#include "fullyprobe.h"

#include <math.h>
#include <stdlib.h>
#include <unistd.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "board.h"
#include "logicSolve.h"
#include "solver.h"

using namespace std;

std::vector<TempC> VtempC;
std::vector<TempC> changeC;
std::vector<TempC> newupixels;
uint64_t oldData[50];

bool cmp(const TempC& a, const TempC& b) {
	// return a.cal<b.cal ;    //左到右  小到大 (左移)
	return a.cal > b.cal; // 左到右  大到小 (右移)
}

int fp2(FullyProbe& fp, LogicSolve& ls, Board& board) {
	RLmost_init(ls, board);
	int res = Logic_solve(ls, board);

	fp.PropagateCount = debugBoard(board);
	ls.solvedBoard = board;

	if (res != INCOMP) return res;

	Dual_for(i, j) fp.gp[i][j][0] = fp.gp[i][j][1] = board;

	Dual_for(i, j) if (getBit(board, i, j) == BIT_UNKNOWN) {
		fp.P.insert(i * 25 + j);
		setBit(fp.gp[i][j][0], i, j, BIT_ZERO);
		setBit(fp.gp[i][j][1], i, j, BIT_ONE);
	}

	while (1) {
		int p = fp.P.begin();
		if (p == -1) {
			if (fp.oldP.isEmpty())
				break;
			else {
				fp.P = fp.oldP;
				fp.oldP.clear();
				continue;
			}
		}

		if (getBit(board, p / 25, p % 25) == BIT_UNKNOWN) {
			res = probe(fp, ls, board, p / 25, p % 25);
			if (res == SOLVED || res == CONFLICT) {
				return res;
			}
		}
	}

	getSize(board);

	fp.mainBoard = board;
	setBestPixel(fp, board);
	return INCOMP;
}

void setBestPixel(FullyProbe& fp, Board& board) {
	// auto max = make_tuple(0,0,0);
	int max[3] = {0, 0, 0};
	double maxPixel = -9E10;

	Dual_for(i, j) if (getBit(board, i, j) == BIT_UNKNOWN) {
		for (int k = 0; k < 50; ++k) {
			fp.gp[i][j][0].data[k] &= board.data[k];
			fp.gp[i][j][1].data[k] &= board.data[k];
		}

		getSize(fp.gp[i][j][1]);
		getSize(fp.gp[i][j][0]);

		double ch = choose(fp.method, fp.gp[i][j][1].size - board.size,
		                   fp.gp[i][j][0].size - board.size);

		if (ch > maxPixel) {
			// max = make_tuple(i,j, fp.gp[i][j][0].size > fp.gp[i][j][1].size ?
			// 0 : 1);
			max[0] = i;
			max[1] = j;
			if (fp.setting == 0) {
				max[2] = fp.gp[i][j][0].size < fp.gp[i][j][1].size ? 0 : 1;
			} else {
				max[2] = fp.gp[i][j][0].size > fp.gp[i][j][1].size ? 0 : 1;
			}
			maxPixel = ch;
		}
	} // big if end

	// printf("select %d %d %lf\n" , get<0>(max) , get<1>(max) , maxPixel );
	fp.max_g0 = fp.gp[max[0]][max[1]][max[2]];
	fp.max_g1 = fp.gp[max[0]][max[1]][!max[2]];
}

#define vlog(x) (log(x + 1) + 1)
double choose(int method, double mp1, double mp0) {
	switch (method) {
		case CH_SUM:
			return mp1 + mp0;
			break;
		case CH_MIN:
			return min(mp1, mp0);
			break;
		case CH_MAX:
			return max(mp1, mp0);
			break;
		case CH_MUL:
			return ++mp1 * ++mp0;
			break;
		case CH_SQRT:
			return min(mp1, mp0) + sqrt(max(mp1, mp0) / (min(mp1, mp0) + 1));
			break;
		case CH_MIN_LOGM:
			return min(mp1, mp0) + vlog(mp1) * vlog(mp0);
			break;
		case CH_MIN_LOGD:
			return min(mp1, mp0) + abs(vlog(mp1) - vlog(mp0));
			break;
		case CH_NEW_MIN_LOGD:
			return min(mp1, mp0) + vlog(abs(mp0 - mp1) + 1);
			break;
		default:
			return ++mp1 * ++mp0;
	}
}

int probe(FullyProbe& fp, LogicSolve& ls, Board& board, int pX, int pY) {
	for (int i = 0; i < 50; ++i) {
		fp.gp[pX][pY][0].data[i] &= board.data[i];
		fp.gp[pX][pY][1].data[i] &= board.data[i];
	}

	int p0 = probeG(fp, ls, pX, pY, BIT_ZERO);
	if (p0 == SOLVED) {
		return SOLVED;
	}

	int p1 = probeG(fp, ls, pX, pY, BIT_ONE);
	if (p1 == SOLVED) {
		return SOLVED;
	}

	if (p0 == CONFLICT && p1 == CONFLICT) {
		return CONFLICT;
	} else if (p1 == CONFLICT) {
		board = fp.gp[pX][pY][0];
	} else if (p0 == CONFLICT) {
		board = fp.gp[pX][pY][1];
	} else {
		for (int i = 0; i < 50; ++i) {
			board.data[i] = fp.gp[pX][pY][0].data[i] | fp.gp[pX][pY][1].data[i];
		}
	}
	return INCOMP;
}

int probeG(FullyProbe& fp, LogicSolve& ls, int pX, int pY, uint64_t pVal) {
	pVal -= BIT_ZERO;
	Board newG = fp.gp[pX][pY][pVal];

	int newGstate = Logic_solve(ls, newG);
	if (newGstate == SOLVED || newGstate == CONFLICT) return newGstate;

	fp.gp[pX][pY][pVal] = newG;

	return newGstate;
}
