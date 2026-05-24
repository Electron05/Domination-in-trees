#include <iostream>
#include <fstream>
#include <string>
#include <vector> 

std::ifstream plik("src/nauty/graphs/my_trees.txt");

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