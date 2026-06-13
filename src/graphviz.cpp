#include <string>
#include <sstream>
#include <vector>
#include <tuple>
#include "tree.h"
#include <fstream>


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

void writeDotFile(const std::string& variant, const Tree& graph, const std::vector<int>& dominatingSet) {
	std::string dotString = generateDotString(graph, dominatingSet);
	std::string filePath = "src/nauty/graphs/" + variant + "_viz.dot";
	std::ofstream outFile(filePath);
	if (outFile.is_open()) {
		outFile << dotString;
		outFile.close();
	}
}

void writeDotFile(const std::string& variant, const Tree& graph, const std::vector<int>& selfDefendedSet, const std::vector<int>& defendingSet) {
	std::string dotString  = generateDotStringRoman(graph, selfDefendedSet, defendingSet);
	std::string filePath = "src/nauty/graphs/" + variant + "_viz.dot";
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
				for (int c = 0; c < assigned.size(); c++) {
					if (c > 0) dot << ":";
					dot << RAINBOW_PALETTE[assigned[c] % RAINBOW_PALETTE.size()];
				}
				dot << "\"";
			}
			dot << ", label=\"" << i << "\\n{";
			for (int c = 0; c < assigned.size(); c++) {
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

void writeDotFileKRainbow(const Tree& graph, int k, const std::vector<BitMask>& colorMask) {
	std::string dotString = generateDotStringKRainbow(graph, k, colorMask);
	std::string filePath = "src/nauty/graphs/krainbow_viz.dot";
	std::ofstream outFile(filePath);
	if (outFile.is_open()) {
		outFile << dotString;
		outFile.close();
	}
}