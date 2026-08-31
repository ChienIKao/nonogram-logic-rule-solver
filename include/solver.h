#ifndef SOLVER_H
#define SOLVER_H

#include "fullyprobe.h"
#include "logicSolve.h"

class Solver {
	public:
		Solver();

		int doSolve(int *data);
		int Solve(int *data);
		void dfs(FullyProbe &fp, LogicSolve &ls, Board &board, int depth);
		void dfs_stack(FullyProbe &fp, LogicSolve &ls, Board b, int depth);

		void setMethod();
		Board getSolvedBoard() { return ls.solvedBoard; }

		int pixel = 0;
		int probNum = -1;

	private:
		int method;
		LogicSolve ls;
		FullyProbe fp;

		int dfs_times = 0;
		int max_dfs_depth = 0;
		int flag = 0;
		int thres;
		int sw;
		int depth_rec[626];
		bool finish = false;
		Board original_b;
};

#endif
