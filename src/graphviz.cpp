#include <string>
#include <sstream>
#include <vector>
#include <tuple>
#include "tree.h"
#include <fstream>

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

void writeDotFile(const std::string& variant, const Tree& graph, const std::vector<int>& dominatingSet) {
	std::string dotString = generateDotString(graph, dominatingSet);
	std::string filePath = "src/nauty/graphs/" + variant + "_viz.dot";
	std::ofstream outFile(filePath);
	if (outFile.is_open()) {
		outFile << dotString;
		outFile.close();
	}
}