#include "benchmark.h"
#include "brute_force.h"
#include "graphviz.h"

int main() {
	std::string outputDir = createOutputFolder();
	runBenchmarks(outputDir);
	runBruteForceTests(outputDir);
	generatePreviews(outputDir);
	return 0;
}
