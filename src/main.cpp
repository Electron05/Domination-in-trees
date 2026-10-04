#include "benchmark.h"
#include "graphviz.h"

int main() {
	std::string outputDir = createOutputFolder();
	runBenchmarks(outputDir);
	generatePreviews(outputDir);
	return 0;
}