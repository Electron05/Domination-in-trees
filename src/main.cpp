#include <iostream>
#include <fstream>
#include <string>
#include <vector> 
#include <tuple>
#include <queue>
#include <sstream>


// Converts a tree represented as an undirected edge list into 
// a topologically sorted parent array.

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

std::vector<int> edgesToParentArray(const std::vector<std::tuple<int, int>>& edges) {
	int n = edges.size() + 1;

	std::vector<std::vector<int>> neighbour_list(n);
	for (auto& edge : edges) {
		int u = std::get<0>(edge);
		int v = std::get<1>(edge);
		neighbour_list[u].push_back(v);
		neighbour_list[v].push_back(u);
	}

	std::vector<bool> visited(n, false);
	// -2 = init -1 = root
	std::vector<int> parent_array(n, -2);
	std::vector<int> old_to_new(n, -1);
	std::queue<int> q;

	visited[0] = true;
	parent_array[0] = -1;
	old_to_new[0] = 0;
	q.push(0);

	int new_id = 0;

	while (!q.empty()) {
		int cur_old = q.front();
		q.pop();
		
		int par_id = old_to_new[cur_old];

		for (int neighbour : neighbour_list[cur_old]) {
			if (!visited[neighbour]) {
				new_id++;
				q.push(neighbour);
				visited[neighbour] = true;
				old_to_new[neighbour] = new_id;
				parent_array[new_id] = par_id;
			}
		}
	}

	return parent_array;
}

void openNthTree(std::ifstream& plik, int n, std::vector<std::tuple<int, int>>& edges) {
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
            edges.push_back({u, v});
        }
    }
}

int main() {
	std::ifstream plik("src/nauty/graphs/my_trees.txt");

	if (!plik.is_open()) {
		return 1;
	}

	std::vector<std::tuple<int, int>> treeEdges;

	openNthTree(plik, 0, treeEdges);

	std::vector<int> parentArray = edgesToParentArray(treeEdges);

	std::cout << "Parent Array:\n[ ";
	for (int i = 0; i < parentArray.size(); i++) {
		std::cout << parentArray[i] << " ";
	}
	std::cout << "]\n";

	return 0;
}