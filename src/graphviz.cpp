#include <string>
#include <sstream>
#include <vector>
#include <tuple>
#include "tree.h"

std::string generateDotString(const Tree& graph, const std::vector<int>& dominatingSet) {
	std::stringstream dot;
	
	dot << "graph G {\n";
	
	dot << "    overlap=\"false\";\n";
	dot << "    splines=\"true\";\n";
	dot << "    node [style=\"filled\", fillcolor=\"white\", fontcolor=\"black\"];\n\n";

	for (int node : dominatingSet) {
		dot << "    " << node << " [fillcolor=\"orange\"];\n";
	}
	
	dot << "\n";

	for (const auto& edge : graph.edgeList) {
		int u = std::get<0>(edge);
		int v = std::get<1>(edge);
		dot << "    " << u << " -- " << v << ";\n";
	}

	dot << "}\n";
	
	return dot.str();
}