#include "helper.h"

Helper::Helper() {
	problemStart = 1;
	problemEnd = 1000;
	method = CH_MUL;
	memset(inputFileName, 0, sizeof(inputFileName));
	memset(outputFileName, 0, sizeof(outputFileName));
	memset(logFileName, 0, sizeof(logFileName));
	strcpy(inputFileName, INPUT_FILE_NAME);
	strcpy(outputFileName, OUTPUT_FILE_NAME);
	strcpy(logFileName, LOG_FINLE_NAME);

	clearFile(outputFileName);
	clearFile(logFileName);
}

int Helper::readOptions(int argc, char** argv) {
	bool showResult = false;
	for (int i = 1; i < argc; ++i) {
		if (i + 1 < argc &&
		    (!strcmp(argv[i], "-S") || !strcmp(argv[i], "--start"))) {
			int n = atoi(argv[i + 1]);
			problemStart = n;
			i++;
			continue;
		}

		if (i + 1 < argc &&
		    (!strcmp(argv[i], "-E") || !strcmp(argv[i], "--end"))) {
			int n = atoi(argv[i + 1]);
			problemEnd = n;
			i++;
			continue;
		}

		if (i + 1 < argc &&
		    (!strcmp(argv[i], "-I") || !strcmp(argv[i], "--input"))) {
			strncpy(inputFileName, argv[i + 1], 512);
			i++;
			continue;
		}

		if (i + 1 < argc &&
		    (!strcmp(argv[i], "-O") || !strcmp(argv[i], "--output"))) {
			strncpy(outputFileName, argv[i + 1], 512);
			i++;
			continue;
		}

		if (i + 1 < argc &&
		    (!strcmp(argv[i], "-L") || !strcmp(argv[i], "--log"))) {
			strncpy(logFileName, argv[i + 1], 100);
			i++;
			continue;
		}

		if (i + 1 < argc &&
		    (!strcmp(argv[i], "-M") || !strcmp(argv[i], "--method"))) {
			int n = atoi(argv[i + 1]);
			if (n < 1 || n > 7) n = CH_MUL;
			method = n;
			i++;
			continue;
		}

		if (!strcmp(argv[i], "--show-config")) {
			showResult = true;
			continue;
		}

		// all options are not fit
		printUsage(argv[0]);
		return 0;
	}

	print(showResult);

	return 1;
}

void Helper::printUsage(const char* name) {
	printf("%s [options]\n", name);
	printf("  -S N\n");
	printf("  --start N\n");
	printf("    start from problem N\n");

	printf("  -E N\n");
	printf("  --end N\n");
	printf("    end by problem N\n");

	printf("  -I [file]\n");
	printf("  --input [file]\n");
	printf("    set input file, default:input.txt\n");

	printf("  -O [file]\n");
	printf("  --output [file]\n");
	printf("    set output file, default:output.txt\n");

	printf("  -L [file]\n");
	printf("  --log [file]\n");
	printf("    set log file name\n");

	printf("  -M n\n");
	printf("  --method n\n");
	printf("    set choosing method 1~7\n");
}

void Helper::clearFile(const char* s) {
	FILE* f = fopen(s, "w");
	fclose(f);
}

int* Helper::allocMem(int n) {
	int* ptr = new int[n];
	return ptr;
}
