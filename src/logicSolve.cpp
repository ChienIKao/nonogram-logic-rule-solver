#include "logicSolve.h"

#include <hash.h>
#include <stdio.h>

#include <cstring>

int left[25], right[25];

LogicSolve::LogicSolve() { init_change(); }

void LogicSolve::load(int* data) {
	for (int i = 0; i < 50; i++) {
		clue[i].count = data[i * 14];

		for (int j = 1; j <= data[i * 14]; j++) {
			clue[i].num[j - 1] = data[i * 14 + j];
		}
	}
}

void LogicSolve::init_change() {
	for (int i = 0; i < 50; i++) changedLine[i] = 1;
}

int Propagate(LogicSolve& ls, Board& board) {
	uint64_t chkline = 0;
	for (int i = 0; i < 50; ++i) {
		if (board.oldData[i] != board.data[i]) chkline |= 1LL << i;
	}
	uint64_t nextchk = 0LL;
	int lineNum = 0;

	while (1) {
		if (chkline == 0) {
			if (nextchk == 0) break;
			chkline = nextchk;
			nextchk = 0LL;
		}

		lineNum = __builtin_ffsll(chkline) - 1;
		chkline &= chkline - 1;

		uint64_t line = getLine(board, lineNum) << 4;

		__SET(line, 1, BIT_ZERO);
		uint64_t newLine = 0LL;

		// cache
		if (!findHash(ls.clue[lineNum], line, newLine)) {
			newLine = 0LL;

			const int state = Logic_rule(ls, board);
			if (state == CONFLICT) {
				return CONFLICT;
			}

			insertHash(ls.clue[lineNum], line, newLine);
		}
		// cache
		newLine >>= 4;
		line >>= 4;

		if (line != newLine) {
			board.data[lineNum] = newLine;

			uint64_t p = line ^ newLine;
			int x = 1;

			while (x = __builtin_ffsll(p), x-- != 0) {
				uint64_t bit = (x & 0x1) == 0 ? BIT_ONE : BIT_ZERO;
				p &= p - 1;
				x >>= 1;

				if (lineNum < 25) {
					nextchk |= 0x1LL << (x + 25);
					__SET(board.data[x + 25], lineNum, bit);
				} else {
					nextchk |= 0x1LL << x;
					__SET(board.data[x], (lineNum - 25), bit);
				}
			}
		}
	}
	memcpy(board.oldData, board.data, sizeof(board.oldData));
	if (unlikely(getSize(board) != 625)) {
		return INCOMP;
	}
	ls.solvedBoard = board;
	return SOLVED;
}

int Logic_solve(LogicSolve& ls, Board& board) {
	int state = INCOMP;
	ls.init_change();

	while (state == INCOMP) {
		memcpy(board.oldData, board.data, sizeof(board.oldData));
		state = Logic_rule(ls, board);
	}

	if (state == CONFLICT) return CONFLICT;

	memcpy(board.oldData, board.data, sizeof(board.oldData));

	if (unlikely(getSize(board) != 625)) return INCOMP;

	ls.solvedBoard = board;
	return SOLVED;
}

int RLmost_init(LogicSolve& ls, Board& board) {
	for (int lineNum = 0; lineNum < 50; lineNum++) {
		uint64_t line = getLine(board, lineNum);
		uint64_t newLine = FILL;

		int clueCnt = ls.clue[lineNum].count;
		int totalClueLen = 0;
		for (int i = 0; i < clueCnt; i++) {
			totalClueLen += ls.clue[lineNum].num[i];
		}
		totalClueLen = totalClueLen + clueCnt - 1;

		int shift = 25 - totalClueLen;
		int point = shift;
		for (int i = 0; i < clueCnt; i++) {
			if (shift < ls.clue[lineNum].num[i]) {
				int paint = ls.clue[lineNum].num[i] - shift;
				for (int p = point; p < point + paint; p++) {
					__SET(line, p, BIT_ONE);
				}
			}
			point += ls.clue[lineNum].num[i] + 1;
		}

		for (int i = 0; i < 25; i++) {
			if (__GET(line, i) != __GET(board.data[lineNum], i)) {
				if (lineNum < 25) {
					__SET(board.data[i + 25], lineNum, __GET(line, i));
				} else {
					__SET(board.data[i], (lineNum - 25), __GET(line, i));
				}
			}
		}
		board.data[lineNum] = line;
	}
	memcpy(board.oldData, board.data, sizeof(board.oldData));
	if (unlikely(getSize(board) != 625)) return INCOMP;
	ls.solvedBoard = board;
	return SOLVED;
}

int RLmost(LogicSolve& ls, int lineNum, const uint64_t& line) {
	for (int i = 0; i < 25; i++) {
		left[i] = 2;
		right[i] = 2;
	}

	bool finish = false;
	int clueCnt = ls.clue[lineNum].count, currClue = 0, startPos = 0;
	while (!finish) {
		int totalClueLen = 0; // 從當前線索開始的所有線索的最小長度(間隔一格)
		for (int i = currClue; i < clueCnt; i++) {
			totalClueLen += ls.clue[lineNum].num[i];
		}
		totalClueLen += clueCnt - currClue - 1;

		// CONFLICT RULE
		// 從當前 index 開始放置線索會超出範圍時，代表 CONFLICT
		if (startPos + totalClueLen > 25) return CONFLICT;

		int state = Update_leftmost(ls, lineNum, line, startPos, currClue);
		if (state == CONFLICT) return CONFLICT;

		finish = true;

		// 檢查每個位置是否符合條件
		for (int i = 0; i < 25; i++) {
			if (__GET(line, i) == BIT_ONE) {
				// line[i] 在 clue 之間
				// Ex. Clue : [1, 2, 3]
				//     line : x x x x x 1 x x x 1
				// leftMost : 1 x 1 1 x x x 1 1 1
				// -------------------------------
				// => final : 1 x x x 1 1 x 1 1 1
				int j;
				for (j = clueCnt - 1; j > 0; j--) {
					if (ls.leftMost[lineNum][j - 1].t < i &&
					    i < ls.leftMost[lineNum][j].h) {
						finish = false;
						break;
					}
				}

				// 檢查 clue 是否在 line[i] 之前
				// Ex. Clue : [1, 2]
				//     line : x x x x x x x x 1 x
				// leftMost : 1 x 1 1 x x x x x x
				// -------------------------------
				// => final : 1 x x x x x x 1 1 x
				if (ls.leftMost[lineNum][clueCnt - 1].t < i) {
					finish = false;
					j = clueCnt;
				}

				// 如果未完成，從 start 開始重新計算 leftmost
				if (!finish) {
					startPos = ls.leftMost[lineNum][j - 1].h + 1;
					currClue = j - 1;
					break;
				}
			}
		}
	}

	// 將 Pixel 的範圍寫入 left
	// Ex. Clue : [1, 2]
	//     line : x x x x x x x x 1 x
	// ==> left : 3 3 x x x x x 4 4 x
	for (int i = 0; i < clueCnt; i++) {
		for (int j = ls.leftMost[lineNum][i].h; j <= ls.leftMost[lineNum][i].t;
		     j++) {
			left[j] = i + 3;
		}
	}

	finish = false, currClue = clueCnt - 1, startPos = 24;
	while (!finish) {
		int totalClueLen = 0; // 從當前線索開始的所有線索的最小長度(間隔一格)
		for (int i = 0; i <= currClue; i++) {
			totalClueLen += ls.clue[lineNum].num[i] + 1;
		}
		totalClueLen--;

		// CONFLICT RULE
		// 從當前 index 開始放置線索會超出範圍時，代表 CONFLICT
		if (startPos - totalClueLen < -1) return CONFLICT;

		int state = Update_rightmost(ls, lineNum, line, startPos, currClue);
		if (state == CONFLICT) return CONFLICT;

		finish = true;

		// 檢查每個位置是否符合條件
		for (int i = 24; i >= 0; i--) {
			if (__GET(line, i) == BIT_ONE) {
				int j;
				for (j = 1; j < clueCnt; j++) {
					if (ls.rightMost[lineNum][j - 1].t < i &&
					    i < ls.rightMost[lineNum][j].h) {
						finish = false;
						break;
					}
				}

				if (ls.rightMost[lineNum][0].h > i) {
					finish = false;
					j = 0;
				}

				// 如果未完成，從 start 開始重新計算 leftmost
				if (!finish) {
					startPos = ls.rightMost[lineNum][j].t - 1;
					currClue = j;
					break;
				}
			}
		}
	}

	for (int i = 0; i < clueCnt; i++) {
		for (int j = ls.rightMost[lineNum][i].h;
		     j <= ls.rightMost[lineNum][i].t; j++) {
			right[j] = i + 3;
		}
	}

	int from = 0, leftCnt, rightCnt;
	for (int i = 0; i < clueCnt; i++) {
		int c = i + 3;
		leftCnt = 0, rightCnt = 0;
		for (int j = from; j < 25; j++) {
			int currClueLen = ls.clue[lineNum].num[i];
			if (left[j] == c) {
				leftCnt++;
				if (leftCnt == currClueLen) from = j;
			}

			if (right[j] == c) rightCnt++;

			// CONFLICT RULE
			// 超過當前線索長度，代表 CONFLICT
			if (leftCnt > currClueLen || rightCnt > currClueLen)
				return CONFLICT;

			// CONFLICT RULE
			// 線索對應錯誤，代表 CONFLICT
			if ((left[j] > c && leftCnt < currClueLen) ||
			    (right[j] > c && rightCnt < currClueLen))
				return CONFLICT;
		}
	}
	return SOLVED;
}

int Update_leftmost(LogicSolve& ls, int lineNum, const uint64_t& line,
                    int start, int currClue) {
	int clueCnt = ls.clue[lineNum].count;
	for (int i = start; i < 25, currClue < clueCnt; i++) {
		// CONFLICT RULE
		// 當當前 index 已經是到底了，但是線索還沒放完時，代表 CONFLICT
		if (i == 24 && currClue < clueCnt - 1) return CONFLICT;

		bool error = false;
		int currClueLen = ls.clue[lineNum].num[currClue];

		// 當前線索想填的位置前或後有 1 (代表和其他線索相鄰)
		if ((i - 1 >= 0 && __GET(line, i - 1) == BIT_ONE) ||
		    (i + currClueLen < 25 && __GET(line, i + currClueLen) == BIT_ONE)) {
			continue;
		}

		// 找到合適的區間了但是區間內有 0，代表不是這個位置
		for (int j = i; j < i + currClueLen; j++) {
			if (__GET(line, j) == BIT_ZERO) {
				error = true;
				break;
			}
		}

		if (!error) {
			// printf("linenum = %d, currClue = %d, i = %d\n", lineNum,
			// currClue, i);
			ls.leftMost[lineNum][currClue].h = i;
			ls.leftMost[lineNum][currClue].t = i + currClueLen - 1;
			i = ls.leftMost[lineNum][currClue].t + 1;
			currClue++;
			if (currClue == clueCnt) return 1; // 代表所有線索都更新完了
		}
	}
}

int Update_rightmost(LogicSolve& ls, int lineNum, const uint64_t& line,
                     int start, int currClue) {
	int clueCnt = ls.clue[lineNum].count;
	for (int i = start; i >= 0, currClue >= 0; i--) {
		// CONFLICT RULE
		// 當當前 index 已經是到底了，但是線索還沒放完時，代表 CONFLICT
		if (i == 0 && currClue > 0) return CONFLICT;

		bool error = false;
		int currClueLen = ls.clue[lineNum].num[currClue];

		// 當前線索想填的位置前或後有 1 (代表和其他線索相鄰)
		if ((i + 1 < 25 && __GET(line, i + 1) == BIT_ONE) ||
		    (i - currClueLen >= 0 && __GET(line, i - currClueLen) == BIT_ONE)) {
			continue;
		}

		// 找到合適的區間了但是區間內有 0，代表不是這個位置
		for (int j = i; j > i - currClueLen; j--) {
			if (__GET(line, j) == BIT_ZERO) {
				error = true;
				break;
			}
		}

		if (!error) {
			ls.rightMost[lineNum][currClue].h = i - currClueLen + 1;
			ls.rightMost[lineNum][currClue].t = i;
			i = ls.rightMost[lineNum][currClue].h - 1;
			currClue--;
			if (currClue == -1) return 1; // 代表所有線索都更新完了
		}
	}
}

int Logic_rule(LogicSolve& ls, Board& board) {
	for (int lineNum = 0; lineNum < 50; ++lineNum) {
		if (ls.changedLine[lineNum] != 1) {
			continue;
		}
		uint64_t line = getLine(board, lineNum);
		// uint64_t newLine = ( ( 0x1LL << 50 ) - 0x1LL );
		int clueCnt = ls.clue[lineNum].count;

		int state = RLmost(ls, lineNum, line);

		if (state == CONFLICT) return CONFLICT;

		// 規則 2 4 5
		// 計算每個提示的起點和終點，將範圍內的空格設為0
		// scope_head[0] = 1, scope_tail[0] = 3
		// scope_head[1] = 6, scope_tail[1] = 8
		//  Ex. Clue : [2, 2]
		// -------------------------------
		//      line : x x 1 x x x x 1 x x
		//      left : x 3 3 x x x 4 4 x x
		//     right : x x 3 3 x x x 4 4 x
		// -------------------------------
		// ==> final : 0 x 1 x 0 0 x 1 x 0
		int scope_head[clueCnt], scope_tail[clueCnt];
		int scope_head_cnt[clueCnt], scope_tail_cnt[clueCnt], cnt = 0;
		int final[25];

		for (int i = 0; i < clueCnt; i++) {
			scope_head[i] = 0;
			scope_tail[i] = 0;
			scope_head_cnt[i] = 0;
			scope_tail_cnt[i] = 0;
		}
		for (int i = 0; i < 25; i++) {
			final[i] = 0;
		}

		// 找到每個提示的範圍
		for (int i = 0; i < 25; i++) {
			if (left[i] > 2 && !scope_head_cnt[left[i] - 3]) {
				scope_head[left[i] - 3] = i;
				scope_head_cnt[left[i] - 3] = 1;
			}
			if (right[25 - i - 1] > 2 &&
			    !scope_tail_cnt[right[25 - i - 1] - 3]) {
				scope_tail[right[25 - i - 1] - 3] = 25 - i - 1;
				scope_tail_cnt[right[25 - i - 1] - 3] = 1;
			}
		}

		int currClueLen, head_cnt = 0, tail_cnt = 0;
		for (int i = 0; i < clueCnt; i++) {
			currClueLen = ls.clue[lineNum].num[i];
			for (int j = scope_head[i]; j <= scope_tail[i] - currClueLen + 1;
			     j++) {
				cnt = 0, head_cnt = 0, tail_cnt = 0;
				for (int k = 0; k < currClueLen; k++) {
					if (__GET(line, j + k) == BIT_ZERO) {
						cnt = 1;
						break;
					}
				}

				if (j + currClueLen < 25 &&
				    __GET(line, j + currClueLen) == BIT_ONE) {
					tail_cnt = 1;
				}

				if (j - 1 >= 0 && __GET(line, j - 1) == BIT_ONE) {
					head_cnt = 1;
				}

				if (!cnt && !head_cnt && !tail_cnt) {
					for (int k = 0; k < currClueLen; k++) {
						final[j + k] = 1;
					}
				}
			}
		}

		for (int i = 0; i < 25; i++) {
			if (__GET(line, i) == BIT_UNKNOWN && final[i] == 0) {
				__SET(line, i, BIT_ZERO);
			}
		}

		// RULE1_5
		// Ex. Clue : [3, 4]
		// ---------------------------------
		//  line : x x x 0 x 1 x x x x x x x
		//  left : 3 3 3 0 4 4 4 4 x x x x x
		// right : x x x 0 x 3 3 3 x 4 4 4 4
		// ---------------------------------

		// 計算提示的範圍
		// hPSeg_head[0] = 5
		// hPSeg_tail[0] = 5
		// hPSeg_len[0] = 1

		// 將範圍內的空格設為 1(如果範圍內有 0，則設為 0)
		// 最終: x x x 0 1 1 1 0 x x x x x
		// 最終: x x x 0 0 1 1 0 x x x x x
		// 最終: x x x 0 0 1 1 0 0 x x x x
		// 最終: x x x 0 0 1 1 0 0 x x x x
		// ---------------------------------
		// 將範圍內的空格設為1
		// 最終: x x x 0 x 1 1 x x x x x x
		int hPSeg = 0;
		int hPSeg_head[13];
		int hPSeg_tail[13];
		int hPSeg_len[13];

		// init array
		for (int i = 0; i < 13; i++) {
			hPSeg_head[i] = 0;
			hPSeg_tail[i] = 0;
			hPSeg_len[i] = 0;
		}

		cnt = 0;
		for (int i = 0; i < 25; i++) {
			if (__GET(line, i) == BIT_ONE && !cnt) {
				hPSeg_head[hPSeg] = i;
				cnt = 1;
			} else if (__GET(line, i) != BIT_ONE && cnt) {
				hPSeg_tail[hPSeg] = i - 1;
				hPSeg_len[hPSeg] = i - hPSeg_head[hPSeg];
				hPSeg++;
				cnt = 0;
			} else if (i + 1 == 25 && cnt) {
				hPSeg_tail[hPSeg] = i;
				hPSeg_len[hPSeg] = i - hPSeg_head[hPSeg] + 1;
				hPSeg++;
				cnt = 0;
			}
		}

		for (int i = 0; i < 25; i++) {
			final[i] = 2;
		}

		int h = 0, t = 0;
		for (int i = 0; i < hPSeg; i++) {
			for (int j = 0; j < clueCnt; j++) {
				currClueLen = ls.clue[lineNum].num[j];
				if (scope_head[j] <= hPSeg_head[i] &&
				    scope_tail[j] >= hPSeg_tail[i]) {
					if (scope_tail[j] - currClueLen + 1 >= scope_head[j]) {
						h = scope_tail[j] - currClueLen + 1;
					} else {
						h = scope_head[j];
					}
					if (scope_head[j] + currClueLen - 1 <= scope_tail[j]) {
						t = scope_head[j] + currClueLen - 1;
					} else {
						t = scope_tail[j];
					}
					for (int k = h; k <= t - currClueLen + 1; k++) {
						cnt = false;
						for (int z = k; z < k + currClueLen; z++) {
							if (__GET(line, z) == BIT_ZERO) cnt = true;
						}
						if (!cnt) {
							final[k - 1] = 0;
							final[k + currClueLen] = 0;
							for (int z = k; z < k + currClueLen; z++) {
								if (final != 0) final[z] = 1;
							}
						}
					}
				}
			}
		}

		// merge
		for (int i = 0; i < 25; i++) {
			if (__GET(line, i) == BIT_UNKNOWN && final[i] == 1) {
				__SET(line, i, BIT_ONE);
			}
		}

		for (int i = 0; i < 25; i++) {
			final[i] = 2;
		}

		for (int i = 0; i < hPSeg; i++) {
			if (right[hPSeg_head[i]] != left[hPSeg_head[i]]) {
				int dh = right[hPSeg_head[i]] - 3, dt = left[hPSeg_head[i]] - 3;
				if (dt - dh == 1 &&
				    (dh > 0 && ls.leftMost[lineNum][dh - 1].h ==
				                   ls.rightMost[lineNum][dh - 1].h ||
				     dh == 0)) {
					Pixel ll[2];
					for (int j = ls.leftMost[lineNum][dh].h;
					     j <= ls.rightMost[lineNum][dh].h; j++) {
						if ((j - 1 >= 0 && __GET(line, j - 1) == BIT_ONE) ||
						    (j + ls.clue[lineNum].num[dh] < 25 &&
						     __GET(line, j + ls.clue[lineNum].num[dh]) ==
						         BIT_ONE)) {
							continue;
						}
						for (int m = ls.leftMost[lineNum][dt].h;
						     m <= ls.rightMost[lineNum][dt].h; m++) {
							bool error = false;
							if ((m - 1 >= 0 && __GET(line, m - 1) == BIT_ONE) ||
							    (m + ls.clue[lineNum].num[dt] < 25 &&
							     __GET(line, m + ls.clue[lineNum].num[dt]) ==
							         BIT_ONE) ||
							    (j + ls.clue[lineNum].num[dh] == m) ||
							    (hPSeg_tail[i] < j ||
							     (hPSeg_head[i] >=
							          j + ls.clue[lineNum].num[dh] &&
							      hPSeg_tail[i] < m) ||
							     hPSeg_head[i] >=
							         m + ls.clue[lineNum].num[dt])) {
								continue;
							}
							for (int d = j; d < j + ls.clue[lineNum].num[dh];
							     d++) {
								if (__GET(line, d) == BIT_ZERO) {
									error = true;
									break;
								}
							}
							for (int d = m; d < m + ls.clue[lineNum].num[dt];
							     d++) {
								if (__GET(line, d) == BIT_ZERO) {
									error = true;
									break;
								}
							}
							if (!error) {
								for (int d = j;
								     d < j + ls.clue[lineNum].num[dh]; d++) {
									final[d] = 1;
								}
								for (int d = m;
								     d < m + ls.clue[lineNum].num[dt]; d++) {
									final[d] = 1;
								}
							}
						}
					}

					for (int k = ls.leftMost[lineNum][dh].h;
					     k <= ls.rightMost[lineNum][dh].t; k++) {
						if (__GET(line, k) == BIT_UNKNOWN && final[k] == 2) {
							__SET(line, k, BIT_ZERO);
						}
					}
				}
			}
		}

		// RULE1
		for (int i = 0; i < clueCnt; i++) {
			for (int k = ls.leftMost[lineNum][i].t;
			     k >= ls.rightMost[lineNum][i].h; k--) {
				__SET(line, k, BIT_ONE);
			}
		}

		// RULE3
		int seg_len[clueCnt + 1];
		int seg_place[clueCnt + 1];
		int segment = 0;
		int differ[clueCnt + 1];
		for (int i = 0; i < clueCnt + 1; i++) {
			seg_len[i] = 0;
			seg_place[i] = 0;
			differ[i] = 0;
		}
		cnt = 0;
		for (int i = 1; i < 24; i++) {
			if (__GET(line, i) == BIT_ZERO) {
				if (__GET(line, i - 1) == BIT_ONE && cnt == 0) {
					differ[segment] = 1; // 10
					for (int k = i; k > 0; k--) {
						if (__GET(line, k - 1) != BIT_ONE) {
							seg_place[segment] = k;
							break;
						} else if (k == 1) {
							seg_place[segment] = 0;
						}
					}
					seg_len[segment] = i - seg_place[segment];
					segment++;
				}
				if (__GET(line, i + 1) == BIT_ONE) {
					cnt = 1;
					differ[segment] = 2; // 01
					seg_place[segment] = i + 1;
					for (int k = i; k < 24; k++) {
						if (__GET(line, k + 1) != BIT_ONE) {
							seg_len[segment] = k - seg_place[segment] + 1;
							break;
						} else if (k == 23) {
							seg_len[segment] = 25 - seg_place[segment];
							break;
						}
					}
					segment++;
				}
			} else if (__GET(line, i) == BIT_UNKNOWN && cnt == 1) {
				cnt = 0;
			}
		}
		for (int i = 0; i < segment; i++) {
			int min_clue = 24, head = 0, tail = 0;
			if (differ[i] == 1) { // 10
				for (int k = right[seg_place[i]] - 3;
				     k <= left[seg_place[i]] - 3; k++) {
					if (ls.clue[lineNum].num[k] < min_clue) {
						min_clue = ls.clue[lineNum].num[k];
					}
				}
				tail = seg_place[i] + seg_len[i] - 1;
				head = tail - min_clue + 1;
				cnt = 0;
				for (int k = tail; k >= head; k--) {
					if (__GET(line, k) == BIT_ZERO) {
						cnt = 1;
						break;
					}
				}
				if (cnt == 0) {
					for (int k = head; k <= tail; k++) {
						__SET(line, k, BIT_ONE);
					}
				}
			} else if (differ[i] == 2) { // 01
				for (int k = right[seg_place[i]] - 3;
				     k <= left[seg_place[i]] - 3; k++) {
					if (ls.clue[lineNum].num[k] < min_clue) {
						min_clue = ls.clue[lineNum].num[k];
					}
				}
				head = seg_place[i];
				tail = seg_place[i] + min_clue - 1;
				cnt = 0;
				for (int k = head; k <= tail; k++) {
					if (__GET(line, k) == BIT_ZERO) {
						cnt = 1;
						break;
					}
				}
				if (cnt == 0) {
					for (int k = head; k <= tail; k++) {
						//	printf("head = %d, tail = %d\n", head, tail);
						__SET(line, k, BIT_ONE);
					}
				}
			}
		}

		// EXTRA_PLUS
		for (int i = segment - 1; i >= 0; i--) {
			int stop = 0;
			if (differ[i] == 2) {
				int len = 0;
				// find len
				for (int k = seg_place[i] + seg_len[i]; k < 25; k++) {
					if (__GET(line, k) != BIT_UNKNOWN)
						len++;
					else if (__GET(line, k) != BIT_UNKNOWN) {
						stop = 1;
						break;
					}
				}
				if (stop) break;
				len += seg_len[i];
				int dh = right[seg_place[i]] - 3, dt = left[seg_place[i]] - 3;
				if (dt - dh == 1 &&
				    ls.clue[lineNum].num[dt] <= ls.clue[lineNum].num[dh] &&
				    seg_len[i] != ls.clue[lineNum].num[dh]) {
					if (dt == clueCnt - 1 ||
					    (dt != clueCnt - 1 &&
					     ls.rightMost[lineNum][dt + 1].h ==
					         ls.leftMost[lineNum][dt + 1].h)) {
						__SET(line, seg_place[i] + ls.clue[lineNum].num[dh],
						      BIT_ZERO);
					}
				}
			}
		}

		int hasN = 0;
		for (int i = 0; i < 25; i++) {
			if (__GET(line, i) != __GET(board.data[lineNum], i)) {
				if (lineNum < 25) {
					__SET(board.data[i + 25], lineNum, __GET(line, i));
					ls.changedLine[i + 25] = 1;
				} else {
					__SET(board.data[i], (lineNum - 25), __GET(line, i));
					ls.changedLine[i] = 1;
				}
				hasN = 1;
			}
		}
		if (hasN == 1)
			ls.changedLine[lineNum] = 1;
		else
			ls.changedLine[lineNum] = 0;

		board.data[lineNum] = line;
	}

	for (int i = 0; i < 50; i++) {
		if (board.oldData[i] != board.data[i]) {
			return INCOMP;
		}
	}
	return SOLVED;
}

// int Logic_rule(LogicSolve& ls, Board& board) {
// 	for (int lineNum = 0; lineNum < 50; ++lineNum) {
// 		if (ls.changedLine[lineNum] != 1) {
// 			continue;
// 		}
// 		uint64_t line = getLine(board, lineNum);
// 		// uint64_t newLine = ( ( 0x1LL << 50 ) - 0x1LL );
// 		int j = ls.clue[lineNum].count;

// 		int state = RLmost(ls, lineNum, line);

// 		if (state == CONFLICT) {
// 			return CONFLICT;
// 		}

// 		// RULE 2 4 5

// 		int scope_head[j], scope_tail[j], sh_cont[j], st_cont[j], cont = 0;
// 		int final[25];

// 		for (int i = 0; i < j; i++) {
// 			scope_head[i] = 0;
// 			scope_tail[i] = 0;
// 			sh_cont[i] = 0;
// 			st_cont[i] = 0;
// 		}
// 		for (int i = 0; i < 25; i++) {
// 			final[i] = 0;
// 		}

// 		// find the scope of each clue
// 		for (int i = 0; i < 25; i++) {
// 			if (left[i] > 2 && !sh_cont[left[i] - 3]) {
// 				scope_head[left[i] - 3] = i;
// 				sh_cont[left[i] - 3] = 1;
// 			}
// 			if (right[25 - i - 1] > 2 && !st_cont[right[25 - i - 1] - 3]) {
// 				scope_tail[right[25 - i - 1] - 3] = 25 - i - 1;
// 				st_cont[right[25 - i - 1] - 3] = 1;
// 			}
// 		}
// 		int this_clue, cont_t = 0, cont_h = 0;

// 		for (int i = 0; i < j; i++) {
// 			this_clue = ls.clue[lineNum].num[i];
// 			for (int k = scope_head[i]; k <= scope_tail[i] - this_clue + 1;
// 			     k++) {
// 				cont = 0, cont_t = 0, cont_h = 0;
// 				for (int z = 0; z < this_clue; z++) {
// 					if (__GET(line, k + z) == BIT_ZERO) {
// 						cont = 1;
// 						break;
// 					}
// 				}
// 				if (k + this_clue < 25 &&
// 				    __GET(line, k + this_clue) == BIT_ONE) {
// 					cont_t = 1;
// 				}
// 				if (k - 1 >= 0 && __GET(line, k - 1) == BIT_ONE) {
// 					cont_h = 1;
// 				}
// 				if (!cont && !cont_t && !cont_h) {
// 					for (int z = 0; z < this_clue; z++) {
// 						final[k + z] = 1;
// 					}
// 				}
// 			}
// 		}

// 		for (int i = 0; i < 25; i++) {
// 			if (__GET(line, i) == BIT_UNKNOWN && final[i] == 0) {
// 				__SET(line, i, BIT_ZERO);
// 			}
// 		}

// 		// RULE1_5
// 		int hPSeg = 0;
// 		int hPSeg_head[13];
// 		int hPSeg_tail[13];
// 		int hPSeg_len[13];

// 		// init array
// 		for (int i = 0; i < 13; i++) {
// 			hPSeg_head[i] = 0;
// 			hPSeg_tail[i] = 0;
// 			hPSeg_len[i] = 0;
// 		}
// 		cont = 0;
// 		for (int i = 0; i < 25; i++) {
// 			if (__GET(line, i) == BIT_ONE && !cont) {
// 				hPSeg_head[hPSeg] = i;
// 				cont = 1;
// 			} else if (__GET(line, i) != BIT_ONE && cont) {
// 				hPSeg_tail[hPSeg] = i - 1;
// 				hPSeg_len[hPSeg] = i - hPSeg_head[hPSeg];
// 				hPSeg++;
// 				cont = 0;
// 			} else if (i + 1 == 25 && cont) {
// 				hPSeg_tail[hPSeg] = i;
// 				hPSeg_len[hPSeg] = i - hPSeg_head[hPSeg] + 1;
// 				hPSeg++;
// 				cont = 0;
// 			}
// 		}

// 		for (int i = 0; i < 25; i++) {
// 			final[i] = 2;
// 		}

// 		int h = 0, t = 0;
// 		for (int i = 0; i < hPSeg; i++) {
// 			for (int k = 0; k < j; k++) {
// 				this_clue = ls.clue[lineNum].num[k];
// 				if (scope_head[k] <= hPSeg_head[i] &&
// 				    scope_tail[k] >= hPSeg_tail[i]) {
// 					if (scope_tail[k] - this_clue + 1 >= scope_head[k]) {
// 						h = scope_tail[k] - this_clue + 1;
// 					} else {
// 						h = scope_head[k];
// 					}
// 					if (scope_head[k] + this_clue - 1 <= scope_tail[k]) {
// 						t = scope_head[k] + this_clue - 1;
// 					} else {
// 						t = scope_tail[k];
// 					}
// 					for (int z = h; z <= t - this_clue + 1; z++) {
// 						cont = false;
// 						for (int zz = z; zz < z + this_clue; zz++) {
// 							if (__GET(line, zz) == BIT_ZERO) cont = true;
// 						}
// 						if (!cont) {
// 							final[z - 1] = 0;
// 							final[z + this_clue] = 0;
// 							for (int zz = z; zz < z + this_clue; zz++) {
// 								if (final != 0) final[zz] = 1;
// 							}
// 						}
// 					}
// 				}
// 			}
// 		}

// 		// merge
// 		for (int i = 0; i < 25; i++) {
// 			if (__GET(line, i) == BIT_UNKNOWN && final[i] == 1) {
// 				__SET(line, i, BIT_ONE);
// 			}
// 		}

// 		for (int i = 0; i < 25; i++) {
// 			final[i] = 2;
// 		}

// 		for (int i = 0; i < hPSeg; i++) {
// 			if (right[hPSeg_head[i]] != left[hPSeg_head[i]]) {
// 				int dh = right[hPSeg_head[i]] - 3, dt = left[hPSeg_head[i]] - 3;
// 				if (dt - dh == 1 &&
// 				    (dh > 0 && ls.leftMost[lineNum][dh - 1].h ==
// 				                   ls.rightMost[lineNum][dh - 1].h ||
// 				     dh == 0)) {
// 					Pixel ll[2];
// 					for (int k = ls.leftMost[lineNum][dh].h;
// 					     k <= ls.rightMost[lineNum][dh].h; k++) {
// 						if ((k - 1 >= 0 && __GET(line, k - 1) == BIT_ONE) ||
// 						    (k + ls.clue[lineNum].num[dh] < 25 &&
// 						     __GET(line, k + ls.clue[lineNum].num[dh]) ==
// 						         BIT_ONE)) {
// 							continue;
// 						}
// 						for (int m = ls.leftMost[lineNum][dt].h;
// 						     m <= ls.rightMost[lineNum][dt].h; m++) {
// 							bool error = false;
// 							if ((m - 1 >= 0 && __GET(line, m - 1) == BIT_ONE) ||
// 							    (m + ls.clue[lineNum].num[dt] < 25 &&
// 							     __GET(line, m + ls.clue[lineNum].num[dt]) ==
// 							         BIT_ONE) ||
// 							    (k + ls.clue[lineNum].num[dh] == m) ||
// 							    (hPSeg_tail[i] < k ||
// 							     (hPSeg_head[i] >=
// 							          k + ls.clue[lineNum].num[dh] &&
// 							      hPSeg_tail[i] < m) ||
// 							     hPSeg_head[i] >=
// 							         m + ls.clue[lineNum].num[dt])) {
// 								continue;
// 							}
// 							for (int d = k; d < k + ls.clue[lineNum].num[dh];
// 							     d++) {
// 								if (__GET(line, d) == BIT_ZERO) {
// 									error = true;
// 									break;
// 								}
// 							}
// 							for (int d = m; d < m + ls.clue[lineNum].num[dt];
// 							     d++) {
// 								if (__GET(line, d) == BIT_ZERO) {
// 									error = true;
// 									break;
// 								}
// 							}
// 							if (!error) {
// 								for (int d = k;
// 								     d < k + ls.clue[lineNum].num[dh]; d++) {
// 									final[d] = 1;
// 								}
// 								for (int d = m;
// 								     d < m + ls.clue[lineNum].num[dt]; d++) {
// 									final[d] = 1;
// 								}
// 							}
// 						}
// 					}

// 					for (int k = ls.leftMost[lineNum][dh].h;
// 					     k <= ls.rightMost[lineNum][dh].t; k++) {
// 						if (__GET(line, k) == BIT_UNKNOWN && final[k] == 2) {
// 							__SET(line, k, BIT_ZERO);
// 						}
// 					}
// 				}
// 			}
// 		}

// 		// RULE1
// 		for (int i = 0; i < j; i++) {
// 			for (int k = ls.leftMost[lineNum][i].t;
// 			     k >= ls.rightMost[lineNum][i].h; k--) {
// 				__SET(line, k, BIT_ONE);
// 			}
// 		}

// 		// RULE3
// 		int seg_len[j + 1];
// 		int seg_place[j + 1];
// 		int segment = 0;
// 		int differ[j + 1];
// 		for (int i = 0; i < j + 1; i++) {
// 			seg_len[i] = 0;
// 			seg_place[i] = 0;
// 			differ[i] = 0;
// 		}
// 		cont = 0;
// 		for (int i = 1; i < 24; i++) {
// 			if (__GET(line, i) == BIT_ZERO) {
// 				if (__GET(line, i - 1) == BIT_ONE && cont == 0) {
// 					differ[segment] = 1; // 10
// 					for (int k = i; k > 0; k--) {
// 						if (__GET(line, k - 1) != BIT_ONE) {
// 							seg_place[segment] = k;
// 							break;
// 						} else if (k == 1) {
// 							seg_place[segment] = 0;
// 						}
// 					}
// 					seg_len[segment] = i - seg_place[segment];
// 					segment++;
// 				}
// 				if (__GET(line, i + 1) == BIT_ONE) {
// 					cont = 1;
// 					differ[segment] = 2; // 01
// 					seg_place[segment] = i + 1;
// 					for (int k = i; k < 24; k++) {
// 						if (__GET(line, k + 1) != BIT_ONE) {
// 							seg_len[segment] = k - seg_place[segment] + 1;
// 							break;
// 						} else if (k == 23) {
// 							seg_len[segment] = 25 - seg_place[segment];
// 							break;
// 						}
// 					}
// 					segment++;
// 				}
// 			} else if (__GET(line, i) == BIT_UNKNOWN && cont == 1) {
// 				cont = 0;
// 			}
// 		}
// 		for (int i = 0; i < segment; i++) {
// 			int min_clue = 24, head = 0, tail = 0;
// 			if (differ[i] == 1) { // 10
// 				for (int k = right[seg_place[i]] - 3;
// 				     k <= left[seg_place[i]] - 3; k++) {
// 					if (ls.clue[lineNum].num[k] < min_clue) {
// 						min_clue = ls.clue[lineNum].num[k];
// 					}
// 				}
// 				tail = seg_place[i] + seg_len[i] - 1;
// 				head = tail - min_clue + 1;
// 				cont = 0;
// 				for (int k = tail; k >= head; k--) {
// 					if (__GET(line, k) == BIT_ZERO) {
// 						cont = 1;
// 						break;
// 					}
// 				}
// 				if (cont == 0) {
// 					for (int k = head; k <= tail; k++) {
// 						__SET(line, k, BIT_ONE);
// 					}
// 				}
// 			} else if (differ[i] == 2) { // 01
// 				for (int k = right[seg_place[i]] - 3;
// 				     k <= left[seg_place[i]] - 3; k++) {
// 					if (ls.clue[lineNum].num[k] < min_clue) {
// 						min_clue = ls.clue[lineNum].num[k];
// 					}
// 				}
// 				head = seg_place[i];
// 				tail = seg_place[i] + min_clue - 1;
// 				cont = 0;
// 				for (int k = head; k <= tail; k++) {
// 					if (__GET(line, k) == BIT_ZERO) {
// 						cont = 1;
// 						break;
// 					}
// 				}
// 				if (cont == 0) {
// 					for (int k = head; k <= tail; k++) {
// 						//	printf("head = %d, tail = %d\n", head, tail);
// 						__SET(line, k, BIT_ONE);
// 					}
// 				}
// 			}
// 		}

// 		// EXTRA_PLUS
// 		for (int i = segment - 1; i >= 0; i--) {
// 			int stop = 0;
// 			if (differ[i] == 2) {
// 				int len = 0;
// 				// find len
// 				for (int k = seg_place[i] + seg_len[i]; k < 25; k++) {
// 					if (__GET(line, k) != BIT_UNKNOWN)
// 						len++;
// 					else if (__GET(line, k) != BIT_UNKNOWN) {
// 						stop = 1;
// 						break;
// 					}
// 				}
// 				if (stop) break;
// 				len += seg_len[i];
// 				int dh = right[seg_place[i]] - 3, dt = left[seg_place[i]] - 3;
// 				if (dt - dh == 1 &&
// 				    ls.clue[lineNum].num[dt] <= ls.clue[lineNum].num[dh] &&
// 				    seg_len[i] != ls.clue[lineNum].num[dh]) {
// 					if (dt == j - 1 ||
// 					    (dt != j - 1 && ls.rightMost[lineNum][dt + 1].h ==
// 					                        ls.leftMost[lineNum][dt + 1].h)) {
// 						__SET(line, seg_place[i] + ls.clue[lineNum].num[dh],
// 						      BIT_ZERO);
// 					}
// 				}
// 			}
// 		}

// 		int hasN = 0;
// 		for (int i = 0; i < 25; i++) {
// 			if (__GET(line, i) != __GET(board.data[lineNum], i)) {
// 				if (lineNum < 25) {
// 					__SET(board.data[i + 25], lineNum, __GET(line, i));
// 					ls.changedLine[i + 25] = 1;
// 				} else {
// 					__SET(board.data[i], (lineNum - 25), __GET(line, i));
// 					ls.changedLine[i] = 1;
// 				}
// 				hasN = 1;
// 			}
// 		}
// 		if (hasN == 1)
// 			ls.changedLine[lineNum] = 1;
// 		else
// 			ls.changedLine[lineNum] = 0;

// 		board.data[lineNum] = line;
// 	}

// 	for (int i = 0; i < 50; i++) {
// 		if (board.oldData[i] != board.data[i]) {
// 			return INCOMP;
// 		}
// 	}
// 	return SOLVED;
// }
