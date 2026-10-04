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
#include <filesystem>
#include <iomanip>
#include <ctime>

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

std::string getTimestampFolderName() {
	tzset(); // set timezone
	auto now = std::chrono::system_clock::now();
	std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&nowTime), "%d.%m.%Y_%H-%M-%S");
    return ss.str();
}

inline void writeLog(std::ofstream& logFile, const std::string& msg, bool newline = true) {
	std::cout << msg << (newline ? "\n" : "") << std::flush;
	if (logFile.is_open()) {
		logFile << msg << (newline ? "\n" : "");
		logFile.flush();
	}
}
}

std::string createOutputFolder() {
	std::string runFolder = "src/output/" + getTimestampFolderName();
	std::filesystem::create_directories(runFolder);
	return runFolder;
}

void runBenchmarks(const std::string& outputFolder) {
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

	std::ofstream logFile(outputFolder + "/benchmark_log.txt");

	std::vector<VariantInfo> variants = {
		{"Total domination", VariantType::TOTAL, 0},
		{"Restrained domination", VariantType::RESTRAINED, 0},
		{"2-domination", VariantType::TWO_DOMINATION, 0},
		{"Roman domination", VariantType::ROMAN, 0}
	};
	for (int k : kValues) {
		variants.push_back({
			std::to_string(k) + "-Rainbow domination",
			VariantType::K_RAINBOW,
			k
		});
	}

	size_t maxLen = 0;
	for (const auto& var : variants) {
		maxLen = std::max(maxLen, var.name.length());
	}

	std::vector<int> timeRow(treeSizes.size(), 0);
	std::vector<std::vector<int>> times(variants.size(), timeRow);
	// solving treesPerSize trees of sizes[j] for dominating variant[i] takes times[i][j] milliseconds
	
	for (size_t sizeIndex = 0; sizeIndex < treeSizes.size(); sizeIndex++) {
		writeLog(logFile, "\n==============================");
		writeLog(logFile, "Tree size V = " + std::to_string(treeSizes[sizeIndex]) + " (" + std::to_string(treesPerSize) + " trees)");
		writeLog(logFile, "==============================");
		
		auto t0 = std::chrono::steady_clock::now();
		std::vector<Tree> treeBatch;
		openTreeBatch(file, (int)sizeIndex, treesPerSize, treeBatch);
		auto t1 = std::chrono::steady_clock::now();
		auto loadTime = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
		writeLog(logFile, "Load time: " + std::to_string(loadTime) + " ms");
		
		for (size_t variantIndex = 0; variantIndex < variants.size(); variantIndex++) {
			const auto& variant = variants[variantIndex];
			std::string padding(maxLen >= variant.name.length() ? maxLen - variant.name.length() : 0, ' ');
			writeLog(logFile, " - " + variant.name + ": " + padding, false);
			
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
			writeLog(logFile, std::to_string(solveTime) + " ms");
			times[variantIndex][sizeIndex] = solveTime;
		}
	}

	size_t maxLabelLen = 0;
	for (int s : treeSizes) {
		std::string sStr = "V = " + std::to_string(s);
		maxLabelLen = std::max(maxLabelLen, sStr.length());
	}

	writeLog(logFile, "\nSummary:");
	for (size_t variantIndex = 0; variantIndex < variants.size(); variantIndex++) {
		writeLog(logFile, variants[variantIndex].name + ":");
		for (size_t sizeIndex = 0; sizeIndex < treeSizes.size(); sizeIndex++) {
			std::string sizeStr = "V = " + std::to_string(treeSizes[sizeIndex]);
			// Right-pad the label so timing columns align neatly based on the longest size label
			std::string sizePadding(maxLabelLen + 2 >= sizeStr.length() ? maxLabelLen + 2 - sizeStr.length() : 1, ' ');
			writeLog(logFile, "  " + sizeStr + ":" + sizePadding + std::to_string(times[variantIndex][sizeIndex]) + " ms");
		}
	}

	std::cout << std::flush;
	std::this_thread::sleep_for(std::chrono::milliseconds(200)); // wait 200ms for docker to print whole output
}