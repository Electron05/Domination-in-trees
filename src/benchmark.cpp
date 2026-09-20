#include "benchmark.h"
#include "tree.h"
#include "total_domination.h"
#include "restrained_domination.h"
#include "2-domination.h"
#include "roman_domination.h"
#include "k-rainbow_domination.h"

#include <vector>
#include <string>
#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <algorithm>

namespace {
const std::vector<int> treeSizes = {512, 1024, 2048, 4096, 8192, 16384};
int treesPerSize = 1000;

void openTreeBatch(std::ifstream& file, 
				   int sizeIndex, 
				   std::vector<Tree>& trees) {

	file.clear();
	file.seekg(0, std::ios::beg);

	int firstTreeIndex = sizeIndex * treesPerSize;
	int currentTreeIndex = -1;
	std::string s;
	s.reserve(90); 

	while (currentTreeIndex < firstTreeIndex) {
		if (!std::getline(file, s)) {
			break;
		}
		if (!s.empty() && s.at(0) == 'G') {
			currentTreeIndex++;
		}
	}

	for (int i = 0; i < treesPerSize; i++) {
		Tree t;
		while (true) {
			std::getline(file, s);
			if (s.empty()) {
				if (i == treesPerSize - 1) break;
				std::getline(file, s); // Skip Graph ..., order ...
				std::getline(file, s); // Skip V, E
				break;
			} 
			
			std::stringstream ss(s);
			int u, v;
			
			while (ss >> u >> v) {
				t.edgeList.push_back({u, v});
			}
		}
		edgesToParentArray(t);
		trees.push_back(t);
	}
}
}

void runBenchmarks() {
	std::string filepath = "src/input/trees/large/large_trees.txt";

	std::ifstream file(filepath);
	if (!file.is_open()) {
		std::cerr << "Error: Could not open file " << filepath << std::endl;
		return;
	}

	std::vector<std::string> variants = {	"Total domination", 
											"Restrained domination",
											"2-domination",
											"Roman domination",
											"3-Rainbow domination",
											"5-Rainbow domination",
											"10-Rainbow domination"};
	size_t maxLen = 0;
	for (const auto& var : variants) {
		maxLen = std::max(maxLen, var.length());
	}

	std::vector<int> timeRow(treeSizes.size(), 0);
	std::vector<std::vector<int>> times(variants.size(), timeRow);
	// solving 1000 trees of sizes[j] for dominating variant[i] takes times[i][j] milliseconds
	
	for (size_t sizeIndex = 0; sizeIndex < treeSizes.size(); sizeIndex++) {
		std::cout << "\n==============================\n";
		std::cout << "Tree size V = " << treeSizes[sizeIndex] << std::endl;
		std::cout << "==============================" << std::endl;
		auto t0 = std::chrono::steady_clock::now();
		std::vector<Tree> treeBatch;
		openTreeBatch(file, (int)sizeIndex, treeBatch);
		auto t1 = std::chrono::steady_clock::now();
		auto loadTime = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
		std::cout << "Load time: " << loadTime << " ms" << std::endl;
		
		for (size_t variantIndex = 0; variantIndex < variants.size(); variantIndex++) {
			std::string padding(maxLen - variants[variantIndex].length(), ' ');
			std::cout << " - " << variants[variantIndex] << ": " << padding << std::flush;
			t0 = std::chrono::steady_clock::now();	
			std::vector<int> dominatingSet;
			std::vector<BitMask> kRainbowDominatingSet;
			std::pair<std::vector<int>, std::vector<int>> romanSets;
			for (int t = 0; t < treesPerSize; t++) {
				switch (variantIndex) {
					case 0:
						dominatingSet = solveTotalDomination(treeBatch[t]);
						break;
					case 1:
						dominatingSet = solveRestrainedDomination(treeBatch[t]);
						break;
					case 2:
						dominatingSet = solve2Domination(treeBatch[t]);
						break;
					case 3:
						romanSets = solveRomanDomination(treeBatch[t]);
						break;
					case 4:
						kRainbowDominatingSet = solveKRainbowDomination(treeBatch[t], 3);
						break;
					case 5:
						kRainbowDominatingSet = solveKRainbowDomination(treeBatch[t], 5);
						break;
					case 6:
						kRainbowDominatingSet = solveKRainbowDomination(treeBatch[t], 10);
						break;
				}
			}
			t1 = std::chrono::steady_clock::now();
			auto solveTime = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
			std::cout << solveTime << " ms" << std::endl;
			times[variantIndex][sizeIndex] = solveTime;
		}
	}

	std::cout << "\nSummary:\n";
	for (size_t variantIndex = 0; variantIndex < variants.size(); variantIndex++) {
		std::cout << variants[variantIndex] << ":\n";
		for (size_t sizeIndex = 0; sizeIndex < treeSizes.size(); sizeIndex++) {
			std::string sizeStr = "V = " + std::to_string(treeSizes[sizeIndex]);
			std::string sizePadding(10 - sizeStr.length(), ' ');
			std::cout << "  " << sizeStr << ":" << sizePadding << times[variantIndex][sizeIndex] << " ms\n";
		}
	}
	std::cout << std::flush;
	std::this_thread::sleep_for(std::chrono::milliseconds(200)); // wait 200ms for docker to print whole output
}