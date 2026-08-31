#ifndef HELPER_H
#define HELPER_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "cdef.h"

class Helper {
	public:
		int problemStart;
		int problemEnd;
		int method;
		char inputFileName[512];
		char outputFileName[512];
		char logFileName[512];

		Helper();

		void print(bool detail) {
			printf("Input file = [%s]\n", inputFileName);

			if (detail) {
				printf("Output file = [%s]\n", outputFileName);
				printf("Problems select from [%d] to [%d]\t", problemStart,
				       problemEnd);
				printf("Method = [%d]\n", method);
			}
		}

		int readOptions(int argc, char** argv);

		void printUsage(const char* name);

		void clearFile(const char* s);

		int* allocMem(int n);
};

#endif
