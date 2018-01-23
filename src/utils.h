/*
 * utils.h
 *
 *  Created on: 3 Jan 2018
 *      Author: mahmoud
 */

//#define LOG2FILE 1
#ifdef __DEBUG__
#ifdef Log2File
#define PRINTF(...) fprintf(f,__VA_ARGS__)
#else
#define PRINTF printf
#endif
#else
#define PRINTF(format, args...) ((void)0)
#endif

#ifndef UTILS_H_
#define UTILS_H_

#define RESULT_DIR string("results")

#define ANSI_COLOR_RESET "\033[0m"
#define RED(TXT)     "\033[1;31m" TXT ANSI_COLOR_RESET
#define GREEN(TXT)   "\033[1;32m" TXT ANSI_COLOR_RESET
#define YELLOW(TXT)  "\033[1;33m" TXT ANSI_COLOR_RESET
#define BLUE(TXT)    "\033[1;34m" TXT ANSI_COLOR_RESET

#include <execinfo.h>
#include <signal.h>
#include <assert.h>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <utils.h>

using namespace std;

void handler(int sig) {
	void* array[10];
	size_t size;

	// get void*'s for all entries on the stack
	size = backtrace(array, 10);

	// print out all the frames to stderr
	fprintf(stderr, "Error: signal %d:\n", sig);
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	exit(1);
}

FILE *f;
void init() {
	signal(SIGSEGV, handler); // install our handler

	/* initialize random seed: */
	srand(time(NULL));
	f = fopen("log.txt", "w");
	if (f == NULL) {
		printf("Error opening file!\n");
		exit(1);
	}
}

template<typename Out>
void split(const std::string &s, char delim, Out result) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		*(result++) = item;
	}
}

std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, std::back_inserter(elems));
	return elems;
}

struct Reporter {
	string name;
	static int counter;
	int index;
	ofstream of;
	ofstream all;
	size_t escapes = 0;

	static string resultDir() {
		string dir = RESULT_DIR + '/';

#ifdef FLUSH_STACK
		dir+="withflush_";
#else
		dir += "noflush_";
#endif

#ifdef doubleTILFA
		dir+="doubleTILFA";
#else
		dir += "TILFA";
#endif
		dir += '/';
		return dir;
	}

	// constructor
	Reporter(const string& path) {
		// prepare result file
		vector<string> tokens = split(path, '/');
		reverse(tokens.begin(), tokens.end());
		name = tokens[1] + '_' + tokens[0];
		string dir = resultDir();
		string resultPath = dir + name + ".txt";

		printf("Result Path: %s\n", resultPath.c_str());

		of.open(resultPath);
		all.open(dir + "all.txt", std::ios_base::app);
		if (!of.is_open() || !all.is_open()) {
			printf("file not opened: %s", resultPath.c_str());
			exit(1);
		}
		index = counter++;
	}
	template<typename T>
	Reporter& operator<<(T input) {
		of << input;
		all << input;
		return *this;
	}
	~Reporter() {
		all.close();
		of.close();
	}
};
int Reporter::counter = 0;

//// what type of graph do you want to use?
typedef PNEANet MyGraph; // undirected graph
//typedef PNGraph PGraph;  //   directed graph
//typedef PNEGraph PGraph;  //   directed multigraph
//typedef TPt<TNodeNet<TInt> > PGraph;
//typedef TPt<TNodeEdgeNet<TInt, TInt> > PGraph;
typedef vector<int> Path;
typedef TNEANet::TEdgeI EdgeI;
typedef TNEANet::TNodeI NodeI;

// iterate over all input files
void forInput(function<void(const string&)> fn) {
	clock_t begin = clock();

	std::ifstream filelist("input_list.txt");
	string path;
	while (filelist >> path) {
		if (path[0] == '#') {
			continue;
		}
		clock_t t = clock();

		fn(path);

		clock_t now = clock();
		clock_t elapsed_secs = double(now - t) / CLOCKS_PER_SEC;
		if (elapsed_secs < 60) {
			PRINTF("elapsed time=%d sec\n", elapsed_secs);
		} else {
			PRINTF("elapsed time=%d min\n", elapsed_secs / 60);
		}
	}

	clock_t end = clock();
	clock_t elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	PRINTF("total time=%.1f min\n", (float) elapsed_secs / 60);
}

#endif /* UTILS_H_ */
