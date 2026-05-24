#include <iostream>
#include <fstream>
#include <string>
#include <vector> 

std::ifstream plik("src/nauty/graphs/my_trees.txt");


/*
neighbour list vector<vector<int>>

vector visited = (n,false)

vector parent array = (n,-2)

vector real to parent array index = (n,-1)


queue q



visited[0] = true

parent array[0] = -1

real to parent array index[0] = 0 

int new_id = 0
while queue.size > 0:
    cur_old = q.dequeue()
    par_id = old_to_new[cur_old]
    for neighbour in neighbour_list[cur_old]: 
        if !visited[neighbour]
            new_id++
            q.enqueue(neighbour)
            visited[neighbour] = true
            old_to_new[neighbour] = new_id
            parent_array[new_id] = par_id*/

void edgesToParentArray(std::string edges){
	int edgeCount = 1;

	int edgesStrLength = edges.length();
	for(int i = 0; i < edgesStrLength-1; i++){
		if(edges[i] == ' ' && edges[i+1] == ' ')
			edgeCount++;
	}

	int n = edgeCount+1;

	std::vector<std::vector<int>> neighbourList(n);

	


	return;
}

void openNthTree(int n){
	int currentTreeIndex = -1;

	std::string s;
	s.reserve(75); 

	while(currentTreeIndex < n){
		std::getline(plik,s);
		if(!s.empty() && s.at(0) == 'G'){
			currentTreeIndex++;
		}
	}
	// Skip V E
	std::getline(plik,s);

	std::string allEdges;
	// Read edges
	while(true){
		std::getline(plik,s);
		if(s.length()<1) break;
		allEdges += s + " ";
		std::cout << "line of edges\n";
	}
	std::cout << allEdges;
	return;
}

int main(){
	std::ifstream plik("src/nauty/graphs/my_trees.txt");

	if(!plik.is_open()){
		return 1;
	}
	openNthTree(10);
	return 0;
}