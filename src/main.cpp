#include "fileManager.h"
#include "helper.h"
#include "solver.h"

#include <ctime>

int main(int argc, char* argv[]) {
	Helper helper;
	if (!helper.readOptions(argc, argv)) {
		printf("\nAborted: Illegal Options.\n");
		return 0;
	}

	int* inputData = helper.allocMem(1001 * 50 * 14);
	readFile(helper.inputFileName, inputData);

	int probData[50 * 14], totalPixel = 0;
	double totalTime = 0.0;
	Solver solver;
	for (int probNum = helper.problemStart; probNum <= helper.problemEnd;
	     probNum++) {
		getData(inputData, probNum, probData);

		solver.probNum = probNum;

		clock_t beginTime = clock();
		if (!solver.doSolve(probData)) {
			printf("Error: $%d solve failed.\n", probNum);
			break;
		}
		clock_t endTime = clock();

		Board ans = solver.getSolvedBoard();

		printf("$%-3d: %3.6lf sec\n", probNum,
		       (double)(endTime - beginTime) / CLOCKS_PER_SEC);
		printBoard(ans, helper.outputFileName, probNum);

		totalPixel += debugBoard(ans);
		totalTime += (double)(endTime - beginTime) / CLOCKS_PER_SEC;
	}
	delete[] inputData;
	printf("Average Pixel: %3.2f\n",
	       totalPixel / (helper.problemEnd - helper.problemStart + 1.0));
	printf("Total Time: %3.6lf sec\n", totalTime);
	return 0;
}
