#include "solver.h"

#include <stdlib.h>

#include <cstdio>
#include <ctime>
#include <vector>

#include "cdef.h"
#include "logicSolve.h"

Solver::Solver() {
	setMethod();
	dfs_times = 0;
	max_dfs_depth = 0;
	finish = false;
}

int first_FP;
int Solver::doSolve(int *data) {
	Board board;
	board.probNum = probNum;

	extern std::vector<TempC> VtempC;

	extern uint64_t oldData[];

	ls.load(data);
	fp.clear();

	clock_t beginTime = clock();
	RLmost_init(ls, board);
	Logic_solve(ls, board);
	clock_t endTime = clock();

	FILE *out = fopen("log.txt", "a+");
	fprintf(out, "$%d\n", board.probNum);
	fprintf(out, "Pixel: %d\n", debugBoard(board));
	fprintf(out, "Propagate Time: %3.10lf sec\n\n",
	        (double)(endTime - beginTime) / CLOCKS_PER_SEC);
	fclose(out);

	VtempC.clear();

	int row[25], column[25];
	for (int i = 0; i < 25; i++) {
		row[i] = 0;
		for (int a = 0; a < 14; a++) {
			row[i] += ls.clue[25 + i].num[a];
		}
	}

	for (int j = 0; j < 25; j++) {
		column[j] = 0;
		for (int a = 0; a < 14; a++) {
			column[j] += ls.clue[j].num[a];
		}
	}

	for (int i = 0; i < 25; i++) {
		for (int j = 0; j < 25; j++) {
			ls.r[i * 25 + j].R = row[i];
			ls.r[i * 25 + j].C = column[j];
			ls.r[i * 25 + j].cal = ls.r[i * 25 + j].C + ls.r[i * 25 + j].R;
		}
	}

	Dual_for(i, j) {
		fp.gp[i][j][0] = fp.u_board;
		fp.gp[i][j][1] = fp.u_board;
	}

	VtempC.clear();
	first_FP = 1;
	if (SOLVED != fp2(fp, ls, board)) {
		finish = false;
		dfs_times = 0;
		thres = 20;
		sw = 0;
		MEMSET_ZERO(depth_rec);
		fp.setting = 0;
		flag = 0;
		original_b = board;
		dfs_stack(fp, ls, board, 0);
	}

	pixel = debugBoard(ls.solvedBoard);

	return 1;
}

int Solver::Solve(int *data) {
	Board board;
	board.probNum = probNum;

	ls.load(data);
	fp.clear();

	clock_t beginTime = clock();
	RLmost_init(ls, board);
	Logic_solve(ls, board);
	clock_t endTime = clock();

	FILE *out = fopen("log.txt", "a+");
	fprintf(out, "$%d\n", board.probNum);
	fprintf(out, "Pixel: %d\n", debugBoard(board));
	fprintf(out, "Propagate Time: %3.10lf sec\n\n",
	        (double)(endTime - beginTime) / CLOCKS_PER_SEC);
	fclose(out);

	if (fp2(fp, ls, board) != SOLVED) {
		finish = false;
		dfs_times = 0;
		max_dfs_depth = 0;
		dfs(fp, ls, board, 0);
	}

	return 1;
}

void Solver::dfs_stack(FullyProbe &fp, LogicSolve &ls, Board b, int depth) {
	dfs_times++;
	extern std::vector<TempC> VtempC;
	extern std::vector<TempC> changeC;
	std::vector<TempC> backup_upixels;
	extern uint64_t oldData[];

	int res;

	depth_rec[depth]++;
	if (first_FP != 1) {
		res = fp2(fp, ls, b);
		if (res == SOLVED) {
			finish = true;
			return;
		}

		if (res == CONFLICT) return;

	} else {
		first_FP = 0;
	}

	Board b1 = fp.max_g1;
	Board b0 = fp.max_g0;

	backup_upixels = VtempC;

	dfs_stack(fp, ls, b0, depth + 1);
	if (finish == true) return;
	VtempC.clear();
	VtempC = backup_upixels;
	backup_upixels.clear();

	dfs_stack(fp, ls, b1, depth + 1);
}

void Solver::dfs(FullyProbe &fp, LogicSolve &ls, Board &board, int depth) {
	if (depth > 625) {
		puts("Aborted: depth > 625");
		exit(1);
	}

	if (depth > max_dfs_depth) max_dfs_depth = depth;

	dfs_times++;

	int res = fp2(fp, ls, board);
	if (res == SOLVED) {
		finish = true;
		return;
	}

	if (res == CONFLICT) {
		return;
	}

	Board b1 = fp.max_g1;
	Board b0 = fp.max_g0;

	dfs(fp, ls, b0, depth + 1);
	if (finish == true) return;

	dfs(fp, ls, b1, depth + 1);
}

void Solver::setMethod() { method = CH_MUL; }
