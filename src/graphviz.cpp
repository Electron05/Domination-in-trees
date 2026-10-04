#include "graphviz.h"
#include "total_domination.h"
#include "restrained_domination.h"
#include "2-domination.h"
#include "roman_domination.h"
#include "k-rainbow_domination.h"
#include <string>
#include <sstream>
#include <vector>
#include <tuple>
#include <fstream>
#include <filesystem>
#include <cstdlib>

namespace {
const std::vector<std::string> RAINBOW_PALETTE = {
	"#ff0000",
	"#0099ff",
	"#00ff00", 
	"#ff00ff",
	"#ffdd00",
	"#ff9900", 
	"#00ffcc",
	"#bd59ff" 
};
}

std::string generateDotString(const Tree& graph, const std::vector<int>& dominatingSet) {
	std::stringstream dot;
	
	dot << "graph G {\n";
	
	dot << "    layout=\"sfdp\";\n";
	dot << "    overlap=\"scale\"; // Fastest overlap calculation\n";
	dot << "    pad=\"2\";\n\n";
	dot << "    splines=\"false\";\n\n";
	dot << "    node [style=\"filled\", fillcolor=\"white\", fontcolor=\"black\"];\n\n";
	dot << "    edge [color=\"black\", penwidth=\"1.0\"];\n\n";

	for (const auto& edge : graph.edgeList) {
		int u = std::get<0>(edge);
		int v = std::get<1>(edge);
		dot << "    " << u << " -- " << v << ";\n";
	}

	dot << "\n";

	for (int node : dominatingSet) {
		dot << "    " << node << " [fillcolor=\"orange\"];\n";
	}

	dot << "}\n";
	
	return dot.str();
}

std::string generateDotStringRoman(const Tree& graph, const std::vector<int>& selfDefendedSet, const std::vector<int>& defendingSet) {
	std::stringstream dot;
	
	dot << "graph G {\n";
	
	dot << "    layout=\"sfdp\";\n";
	dot << "    overlap=\"scale\"; // Fastest overlap calculation\n";
	dot << "    pad=\"2\";\n\n";
	dot << "    splines=\"false\";\n\n";
	dot << "    node [style=\"filled\", fillcolor=\"white\", fontcolor=\"black\"];\n\n";
	dot << "    edge [color=\"black\", penwidth=\"1.0\"];\n\n";

	for (const auto& edge : graph.edgeList) {
		int u = std::get<0>(edge);
		int v = std::get<1>(edge);
		dot << "    " << u << " -- " << v << ";\n";
	}

	dot << "\n";

	for (int node : selfDefendedSet) {
		dot << "    " << node << " [fillcolor=\"" << RAINBOW_PALETTE[5] << "\"];\n";
	}
	for (int node : defendingSet) {
		dot << "    " << node << " [fillcolor=\"" << RAINBOW_PALETTE[0] << "\"];\n";
	}

	dot << "}\n";
	
	return dot.str();
}

void writeDotFile(const std::string& filePath, const Tree& graph, const std::vector<int>& dominatingSet) {
	std::string dotString = generateDotString(graph, dominatingSet);
	std::ofstream outFile(filePath);
	if (outFile.is_open()) {
		outFile << dotString;
		outFile.close();
	}
}

void writeDotFileRoman(const std::string& filePath, const Tree& graph, const std::vector<int>& selfDefendedSet, const std::vector<int>& defendingSet) {
	std::string dotString = generateDotStringRoman(graph, selfDefendedSet, defendingSet);
	std::ofstream outFile(filePath);
	if (outFile.is_open()) {
		outFile << dotString;
		outFile.close();
	}
}

std::string generateDotStringKRainbow(const Tree& graph, int k, const std::vector<BitMask>& colorMask) {
	std::stringstream dot;
	
	dot << "graph G {\n";
	dot << "    layout=\"sfdp\";\n";
	dot << "    overlap=\"scale\";\n";
	dot << "    pad=\"2\";\n\n";
	dot << "    splines=\"false\";\n\n";
	dot << "    node [style=\"filled\", fillcolor=\"white\", fontcolor=\"black\"];\n\n";
	dot << "    edge [color=\"black\", penwidth=\"1.0\"];\n\n";

	for (const auto& edge : graph.edgeList) {
		int u = std::get<0>(edge);
		int v = std::get<1>(edge);
		dot << "    " << u << " -- " << v << ";\n";
	}

	dot << "\n";

	int n = graph.neighbourList.size();
	for (int i = 0; i < n; i++) {
		std::vector<int> assigned;
		for (int c = 0; c < k; c++) {
			if (colorMask[i].test(c)) {
				assigned.push_back(c);
			}
		}

		if (!assigned.empty()) {
			dot << "    " << i << " [";
			if (assigned.size() == 1) {
				std::string col = RAINBOW_PALETTE[assigned[0] % RAINBOW_PALETTE.size()];
				dot << "fillcolor=\"" << col << "\"";
			} else {
				dot << "style=\"filled,wedged\", fillcolor=\"";
				for (size_t c = 0; c < assigned.size(); c++) {
					if (c > 0) dot << ":";
					dot << RAINBOW_PALETTE[assigned[c] % RAINBOW_PALETTE.size()];
				}
				dot << "\"";
			}
			dot << ", label=\"" << i << "\\n{";
			for (size_t c = 0; c < assigned.size(); c++) {
				if (c > 0) dot << ",";
				dot << (assigned[c] + 1);
			}
			dot << "}\"";
			dot << "];\n";
		}
	}

	dot << "}\n";
	
	return dot.str();
}

void writeDotFileKRainbow(const std::string& filePath, const Tree& graph, int k, const std::vector<BitMask>& colorMask) {
	std::string dotString = generateDotStringKRainbow(graph, k, colorMask);
	std::ofstream outFile(filePath);
	if (outFile.is_open()) {
		outFile << dotString;
		outFile.close();
	}
}

void generatePreviews(const std::string& outputFolder) {
	const char* envPreviews = std::getenv("PREVIEWS_PER_VARIANT");
	if (!envPreviews) return;
	int previewsPerVariant = std::atoi(envPreviews);
	if (previewsPerVariant <= 0) return;

	std::string previewFilepath = "src/input/generated_trees/preview_trees.txt";
	std::ifstream pFile(previewFilepath);
	if (!pFile.is_open()) return;

	std::vector<Tree> previewTrees;
	openTreeBatch(pFile, 0, previewsPerVariant, previewTrees);
	if (previewTrees.empty()) return;

	std::vector<int> kValues;
	const char* envK = std::getenv("K_RAINBOW_VALUES");
	if (envK) {
		std::stringstream ss(envK);
		int val;
		while (ss >> val) {
			kValues.push_back(val);
		}
	}

	struct VariantPreview {
		std::string folderName;
		char type;
		int k;
	};

	std::vector<VariantPreview> variants = {
		{"total_domination", 'T', 0},
		{"restrained_domination", 'E', 0},
		{"2-domination", '2', 0},
		{"roman_domination", 'R', 0}
	};
	for (int k : kValues) {
		variants.push_back({std::to_string(k) + "-rainbow_domination", 'K', k});
	}

	for (const auto& var : variants) {
		std::string variantDir = outputFolder + "/" + var.folderName;
		std::filesystem::create_directories(variantDir);

		for (int t = 0; t < (int)previewTrees.size(); t++) {
			std::string dotPath = variantDir + "/" + std::to_string(t + 1) + ".dot";
			switch (var.type) {
				case 'T': {
					auto set = solveTotalDomination(previewTrees[t]);
					writeDotFile(dotPath, previewTrees[t], set);
					break;
				}
				case 'E': {
					auto set = solveRestrainedDomination(previewTrees[t]);
					writeDotFile(dotPath, previewTrees[t], set);
					break;
				}
				case '2': {
					auto set = solve2Domination(previewTrees[t]);
					writeDotFile(dotPath, previewTrees[t], set);
					break;
				}
				case 'R': {
					auto sets = solveRomanDomination(previewTrees[t]);
					writeDotFileRoman(dotPath, previewTrees[t], sets.first, sets.second);
					break;
				}
				case 'K': {
					auto mask = solveKRainbowDomination(previewTrees[t], var.k);
					writeDotFileKRainbow(dotPath, previewTrees[t], var.k, mask);
					break;
				}
			}
		}
	}
}