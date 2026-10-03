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
#include <cstdlib>

namespace {

enum class VariantType : char {
	TOTAL,
	RESTRAINED,
	TWO_DOMINATION,
	ROMAN,
	K_RAINBOW
};

struct VariantInfo {
	std::string name;
	VariantType type;
	int k = 0;
};

std::vector<int> parseIntegerList(const char* envVal) {
	std::vector<int> result;
	if (!envVal) return result;
	std::stringstream ss(envVal);
	int val;
	while (ss >> val) {
		result.push_back(val);
	}
	return result;
}

int parseInt(const char* envVal) {
	if (!envVal) return 0;
	return std::stoi(envVal);

}

void openTreeBatch(std::ifstream& file, 
				   int sizeIndex, 
				   int treesPerSize,
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
	const char* envSizes = std::getenv("TREE_SIZES");
	std::vector<int> treeSizes = parseIntegerList(envSizes);
	if (treeSizes.empty()) {
		std::cerr << "Error: TREE_SIZES environment variable is not defined or invalid." << std::endl;
		return;
	}

	const char* envCount = std::getenv("TREES_PER_BATCH");
	int treesPerSize = parseInt(envCount);
	if (treesPerSize <= 0) {
		std::cerr << "Error: TREES_PER_BATCH environment variable is not defined or invalid." << std::endl;
		return;
	}

	const char* envK = std::getenv("K_RAINBOW_VALUES");
	std::vector<int> kValues = parseIntegerList(envK);
	if (kValues.empty()) {
		std::cerr << "Error: K_RAINBOW_VALUES environment variable is not defined or invalid." << std::endl;
		return;
	}

	std::string filepath = "src/input/generated_trees/generated_trees.txt";

	std::ifstream file(filepath);
	if (!file.is_open()) {
		std::cerr << "Error: Could not open file " << filepath << std::endl;
		return;
	}

	std::vector<VariantInfo> variants = {
		{"Total domination", VariantType::TOTAL, 0},
		{"Restrained domination", VariantType::RESTRAINED, 0},
		{"2-domination", VariantType::TWO_DOMINATION, 0},
		{"Roman domination", VariantType::ROMAN, 0}
	};
	for (int k : kValues) {
		variants.push_back({std::to_string(k) + "-Rainbow domination", VariantType::K_RAINBOW, k});
	}

	size_t maxLen = 0;
	for (const auto& var : variants) {
		maxLen = std::max(maxLen, var.name.length());
	}

	std::vector<int> timeRow(treeSizes.size(), 0);
	std::vector<std::vector<int>> times(variants.size(), timeRow);
	// solving treesPerSize trees of sizes[j] for dominating variant[i] takes times[i][j] milliseconds
	
	for (size_t sizeIndex = 0; sizeIndex < treeSizes.size(); sizeIndex++) {
		std::cout << "\n==============================\n";
		std::cout << "Tree size V = " << treeSizes[sizeIndex] << " (" << treesPerSize << " trees)" << std::endl;
		std::cout << "==============================" << std::endl;
		
		auto t0 = std::chrono::steady_clock::now();
		std::vector<Tree> treeBatch;
		openTreeBatch(file, (int)sizeIndex, treesPerSize, treeBatch);
		auto t1 = std::chrono::steady_clock::now();
		auto loadTime = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
		std::cout << "Load time: " << loadTime << " ms" << std::endl;
		
		for (size_t variantIndex = 0; variantIndex < variants.size(); variantIndex++) {
			const auto& variant = variants[variantIndex];
			std::string padding(maxLen >= variant.name.length() ? maxLen - variant.name.length() : 0, ' ');
			std::cout << " - " << variant.name << ": " << padding << std::flush;
			
			t0 = std::chrono::steady_clock::now();	
			std::vector<int> dominatingSet;
			std::vector<BitMask> kRainbowDominatingSet;
			std::pair<std::vector<int>, std::vector<int>> romanSets;
			for (int t = 0; t < treesPerSize; t++) {
				switch (variant.type) {
					case VariantType::TOTAL:
						dominatingSet = solveTotalDomination(treeBatch[t]);
						break;
					case VariantType::RESTRAINED:
						dominatingSet = solveRestrainedDomination(treeBatch[t]);
						break;
					case VariantType::TWO_DOMINATION:
						dominatingSet = solve2Domination(treeBatch[t]);
						break;
					case VariantType::ROMAN:
						romanSets = solveRomanDomination(treeBatch[t]);
						break;
					case VariantType::K_RAINBOW:
						kRainbowDominatingSet = solveKRainbowDomination(treeBatch[t], variant.k);
						break;
				}
			}
			
			t1 = std::chrono::steady_clock::now();
			auto solveTime = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
			std::cout << solveTime << " ms" << std::endl;
			times[variantIndex][sizeIndex] = solveTime;
		}
	}

	size_t maxLabelLen = 0;
	for (int s : treeSizes) {
		std::string sStr = "V = " + std::to_string(s);
		maxLabelLen = std::max(maxLabelLen, sStr.length());
	}

	std::cout << "\nSummary:\n";
	for (size_t variantIndex = 0; variantIndex < variants.size(); variantIndex++) {
		std::cout << variants[variantIndex].name << ":\n";
		for (size_t sizeIndex = 0; sizeIndex < treeSizes.size(); sizeIndex++) {
			std::string sizeStr = "V = " + std::to_string(treeSizes[sizeIndex]);
			std::string sizePadding(maxLabelLen + 2 >= sizeStr.length() ? maxLabelLen + 2 - sizeStr.length() : 1, ' ');
			std::cout << "  " << sizeStr << ":" << sizePadding << times[variantIndex][sizeIndex] << " ms\n";
		}
	}
	std::cout << std::flush;
	std::this_thread::sleep_for(std::chrono::milliseconds(200)); // wait 200ms for docker to print whole output
}