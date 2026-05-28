#include <iostream>
#include <fstream>
#include <string>
#include <vector> 
#include <tuple>
#include <queue>
#include <sstream>
#include "tree.h"

#include "tree.h"
#include "total_domination.cpp"
#include "restrained_domination.cpp"
#include "graphviz.cpp"

void openNthTree(std::ifstream& plik, int n, Tree& graph) {
	int currentTreeIndex = -1;
	std::string s;
	s.reserve(75); 

	while (currentTreeIndex < n) {
		std::getline(plik, s);
		if (!s.empty() && s.at(0) == 'G') {
			currentTreeIndex++;
		}
	}

	// Skip V E
	std::getline(plik, s);

	while (true) {
		std::getline(plik, s);
		if (s.length() <= 1) break; 
		
		std::stringstream ss(s);
		int u, v;
		
		while (ss >> u >> v) {
			graph.edgeList.push_back({u, v});
		}
	}
}

void printEdgeList(const Tree& graph) {
	std::cout << "Edge List:\n";
	for (const auto& edge : graph.edgeList) {
		std::cout << "(" << std::get<0>(edge) << ", " << std::get<1>(edge) << ")\n";
	}
	std::cout << "\n";
}

void printParentArray(const Tree& graph) {
	std::cout << "Parent Array:\n[ ";
	for (int i = 0; i < graph.parentArray.size(); i++) {
		std::cout << graph.parentArray[i] << " ";
	}
	std::cout << "]\n\n";
}

void printOriginalToParentArrayIndices(const Tree& graph) {
	std::cout << "Mapping (Original Index -> Parent Array Index):\n";
	for (int i = 0; i < graph.parentArrayIndices.size(); i++) {
		std::cout << "Original [" << i << "] -> Parent Array Index [" << graph.parentArrayIndices[i] << "]\n";
	}
	std::cout << "\n";
}

void printDominatingSet(int variant, std::vector<int>& set) {
	std::string s;
	switch (variant){
		case 0:
			s = "Totally Dominating Set:\n";
			break;
		case 1:
			s = "Restrained Dominating Set:\n";
			break;
		default:
			break;
	}
	std::cout << s;
	for(int v : set) {
		std:: cout << v << " ";
	}
	std::cout<< "\n";
}


// Converts a tree represented as an undirected edge list into topologically sorted parent array
// Also maps original vertex indices to corresponding ones ins Parent Array

// 0th node always becomes the root

// Example: A root with 2 children, where one child has its own child.
// Given original edges: (0, 4), (0, 2), (4, 1)
// 1. The BFS queue starts at Old Node 0. It becomes New Node 0 (the root).
// 2. Old 0 connects to Old 4 and Old 2. 
//    Old 4 becomes New 1. (Parent of New 1 is New 0).
//    Old 2 becomes New 2. (Parent of New 2 is New 0).
// 3. Old 4 (now New 1) connects to Old 1. 
//    Old 1 becomes New 3. (Parent of New 3 is New 1).
// 4. Old 2 (now New 2) has no other unvisited connections.
// Resulting Parent Array: [-1, 0, 0, 1]
void edgesToParentArray(Tree& graph) {
	int n = graph.edgeList.size() + 1;

	std::vector<std::vector<int>> neighbourList(n);
	for (const auto& edge : graph.edgeList) {
		int u = std::get<0>(edge);
		int v = std::get<1>(edge);
		neighbourList[u].push_back(v);
		neighbourList[v].push_back(u);
	}

	std::vector<bool> visited(n, false);

	graph.parentArray.assign(n, PARENT_ARRAY_INIT_VALUE);

	graph.parentArrayIndices.assign(n, OLD_TO_NEW_IDEX_INIT);
	graph.orginalIndices.assign(n, OLD_TO_NEW_IDEX_INIT);

	std::queue<int> q;

	visited[0] = true;
	graph.parentArray[0] = PARENT_ARRAY_ROOT_PARENT;
	graph.parentArrayIndices[0] = 0;
	graph.orginalIndices[0] = 0;
	q.push(0);

	int newId = 0;

	while (!q.empty()) {
		int curOld = q.front();
		q.pop();
		
		int parentId = graph.parentArrayIndices[curOld];

		for (int neighbour : neighbourList[curOld]) {
			if (!visited[neighbour]) {
				newId++;
				q.push(neighbour);
				visited[neighbour] = true;
				graph.parentArrayIndices[neighbour] = newId;
				graph.orginalIndices[newId] = neighbour;
				graph.parentArray[newId] = parentId;
			}
		}
	}
}

int main() {
	std::ifstream plik("src/nauty/graphs/my_trees.txt");

	if (!plik.is_open()) {
		return 1;
	}

	Tree myTree;

	openNthTree(plik, 23, myTree);

	printEdgeList(myTree);

	edgesToParentArray(myTree);

	printParentArray(myTree);
	printOriginalToParentArrayIndices(myTree);

	std::vector<int> dominatingSet = solveTotalDomination(myTree);
	printDominatingSet(0,dominatingSet);

	std::string totalDominationDotOutput = generateDotString(myTree, dominatingSet);
	std::ofstream outFile("src/nauty/graphs/total_viz.dot");
	if (outFile.is_open()) {
		outFile << totalDominationDotOutput;
		outFile.close();
		std::cout << "Successfully saved Graphviz data to src/nauty/graphs/total_viz.dot\n\n";
	}

	dominatingSet = solveRestrainedDomination(myTree);
	printDominatingSet(1,dominatingSet);

	std::string restrainedDominationDotOutput = generateDotString(myTree, dominatingSet);
	outFile.clear();
	outFile.open( "src/nauty/graphs/restrained_viz.dot");
	if (outFile.is_open()) {
		outFile << restrainedDominationDotOutput;
		outFile.close();
		std::cout << "Successfully saved Graphviz data to src/nauty/graphs/restrained_viz.dot\n\n";
	}

	return 0;
}