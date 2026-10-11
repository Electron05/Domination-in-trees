#include "tree.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <utility>

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

void openTreeBatch(std::ifstream& file, int sizeIndex, int treesPerSize, std::vector<Tree>& trees) {
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
		trees.push_back(std::move(t));
	}
}

// Reads every tree from the file (showg -e format), trees can have different sizes
// Graph 1, order 4.
// 4 3
// 0 3  1 3  2 3
// (empty line)
void openAllTrees(std::ifstream& file, std::vector<Tree>& trees) {
	std::string s;
	bool insideTree = false;
	Tree t;

	while (std::getline(file, s)) {
		if (!s.empty() && s.at(0) == 'G') {
			std::getline(file, s); // Skip V E
			insideTree = true;
			continue;
		}
		if (!insideTree) continue;

		if (s.empty()) {
			edgesToParentArray(t);
			trees.push_back(std::move(t));
			t = Tree();
			insideTree = false;
			continue;
		}

		std::stringstream ss(s);
		int u, v;
		while (ss >> u >> v) {
			t.edgeList.push_back({u, v});
		}
	}

	// Last tree may not end with an empty line
	if (insideTree) {
		edgesToParentArray(t);
		trees.push_back(std::move(t));
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
	for (size_t i = 0; i < graph.parentArray.size(); i++) {
		std::cout << graph.parentArray[i] << " ";
	}
	std::cout << "]\n\n";
}

void printOriginalToParentArrayIndices(const Tree& graph) {
	std::cout << "Mapping (Original Index -> Parent Array Index):\n";
	for (size_t i = 0; i < graph.parentArrayIndices.size(); i++) {
		std::cout << "Original [" << i << "] -> Parent Array Index [" << graph.parentArrayIndices[i] << "]\n";
	}
	std::cout << "\n";
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

	graph.neighbourList.assign(n, std::vector<int>());
	for (const auto& edge : graph.edgeList) {
		int u = std::get<0>(edge);
		int v = std::get<1>(edge);
		graph.neighbourList[u].push_back(v);
		graph.neighbourList[v].push_back(u);
	}

	std::vector<bool> visited(n, false);

	graph.parentArray.assign(n, PARENT_ARRAY_INIT_VALUE);

	graph.parentArrayIndices.assign(n, OLD_TO_NEW_IDEX_INIT);
	graph.originalIndices.assign(n, OLD_TO_NEW_IDEX_INIT);

	std::queue<int> q;

	visited[0] = true;
	graph.parentArray[0] = PARENT_ARRAY_ROOT_PARENT;
	graph.parentArrayIndices[0] = 0;
	graph.originalIndices[0] = 0;
	q.push(0);

	int newId = 0;

	while (!q.empty()) {
		int curOld = q.front();
		q.pop();
		
		int parentId = graph.parentArrayIndices[curOld];

		for (int neighbour : graph.neighbourList[curOld]) {
			if (!visited[neighbour]) {
				newId++;
				q.push(neighbour);
				visited[neighbour] = true;
				graph.parentArrayIndices[neighbour] = newId;
				graph.originalIndices[newId] = neighbour;
				graph.parentArray[newId] = parentId;
			}
		}
	}
}
