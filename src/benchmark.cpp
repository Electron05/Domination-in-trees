#include "benchmark.h"
#include "tree.h"
#include "total_domination.h"
#include "restrained_domination.h"
#include "2-domination.h"
#include "roman_domination.h"
#include "k-rainbow_domination.h"
#include "memory_tracker.h"
#include "variants.h"
#include "utils.h"

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
std::string getTimestampFolderName() {
	tzset(); // set timezone
	auto now = std::chrono::system_clock::now();
	std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&nowTime), "%d.%m.%Y_%H-%M-%S");
    return ss.str();
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

	const char* envCheck = std::getenv("CHECK_CORRECTNESS");
	bool checkCorrectness = parseInt(envCheck) != 0;

	std::string filepath = "src/input/generated_trees/generated_trees.txt";

	std::ifstream file(filepath);
	if (!file.is_open()) {
		std::cerr << "Error: Could not open file " << filepath << std::endl;
		return;
	}

	std::ofstream logFile(outputFolder + "/benchmark_log.txt");

	std::vector<VariantInfo> variants = createVariants(kValues);

	size_t maxLen = 0;
	for (const auto& var : variants) {
		maxLen = std::max(maxLen, var.name.length());
	}

	std::vector<int> timeRow(treeSizes.size(), 0);
	std::vector<std::vector<int>> times(variants.size(), timeRow);

	std::vector<size_t> memRow(treeSizes.size(), 0);
	std::vector<std::vector<size_t>> peakMem(variants.size(), memRow);
	
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
			
			std::vector<int> dominatingSet;
			std::vector<BitMask> kRainbowDominatingSet;
			std::pair<std::vector<int>, std::vector<int>> romanSets;

			size_t batchPeakMem = 0;

			t0 = std::chrono::steady_clock::now();
			{
				ScopedMemoryTracker memTracker;
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
				batchPeakMem = memTracker.getTotalPeakBytes();
			}
			t1 = std::chrono::steady_clock::now();

			auto solveTime = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
			times[variantIndex][sizeIndex] = solveTime;
			peakMem[variantIndex][sizeIndex] = batchPeakMem;

			double bytesPerVertex = (double)batchPeakMem / treeSizes[sizeIndex];
			std::string timeStr = std::to_string(solveTime) + " ms";
			std::string timePad(timeStr.length() < 8 ? 8 - timeStr.length() : 0, ' ');

			std::string memStr = formatMemory(batchPeakMem);
			std::string memPad(memStr.length() < 9 ? 9 - memStr.length() : 0, ' ');

			std::stringstream bpvStream;
			bpvStream << std::fixed << std::setprecision(1) << "(" << bytesPerVertex << " B/V)";

			std::stringstream memInfo;
			memInfo << timePad << timeStr << " | Peak Mem: "
			        << memPad << memStr
			        << " " << (checkCorrectness ? padRight(bpvStream.str(), 12) : bpvStream.str());

			if (checkCorrectness) {
				int invalidCount = 0;
				for (int t = 0; t < treesPerSize; t++) {
					if (!checkSolution(variant, treeBatch[t]).valid) {
						invalidCount++;
					}
				}
				if (invalidCount == 0) memInfo << " | Valid: OK";
				else memInfo << " | Valid: FAILED (" << invalidCount << "/" << treesPerSize << " trees)";
			}
			writeLog(logFile, memInfo.str());
		}
	}

	size_t maxLabelLen = 0;
	for (int s : treeSizes) {
		std::string sStr = "V = " + std::to_string(s);
		maxLabelLen = std::max(maxLabelLen, sStr.length());
	}

	size_t maxTimeLen = 0;
	size_t maxMemLen = 0;
	for (size_t variantIndex = 0; variantIndex < variants.size(); variantIndex++) {
		for (size_t sizeIndex = 0; sizeIndex < treeSizes.size(); sizeIndex++) {
			std::string tStr = std::to_string(times[variantIndex][sizeIndex]) + " ms";
			maxTimeLen = std::max(maxTimeLen, tStr.length());

			std::string mStr = formatMemory(peakMem[variantIndex][sizeIndex]);
			maxMemLen = std::max(maxMemLen, mStr.length());
		}
	}

	writeLog(logFile, "\nSummary (Execution Time / Peak Memory per Tree):");
	for (size_t variantIndex = 0; variantIndex < variants.size(); variantIndex++) {
		writeLog(logFile, variants[variantIndex].name + ":");
		for (size_t sizeIndex = 0; sizeIndex < treeSizes.size(); sizeIndex++) {
			std::string sizeStr = "V = " + std::to_string(treeSizes[sizeIndex]);
			// Right-pad the label so timing columns align neatly based on the longest size label
			std::string sizePadding(maxLabelLen + 2 >= sizeStr.length() ? maxLabelLen + 2 - sizeStr.length() : 1, ' ');
			
			std::string timeStr = std::to_string(times[variantIndex][sizeIndex]) + " ms";
			std::string timePadding(maxTimeLen >= timeStr.length() ? maxTimeLen - timeStr.length() : 0, ' ');

			size_t mem = peakMem[variantIndex][sizeIndex];
			double bpv = (double)mem / treeSizes[sizeIndex];

			std::string memStr = formatMemory(mem);
			std::string memPadding(maxMemLen >= memStr.length() ? maxMemLen - memStr.length() : 0, ' ');

			std::stringstream ss;
			ss << std::fixed << std::setprecision(1);
			ss << "  " << sizeStr << ":" << sizePadding 
			   << timePadding << timeStr << " | "
			   << memPadding << memStr
			   << " (" << bpv << " B/V)";
			writeLog(logFile, ss.str());
		}
	}

	std::cout << std::flush;
	std::this_thread::sleep_for(std::chrono::milliseconds(200)); // wait 200ms for docker to print whole output
}