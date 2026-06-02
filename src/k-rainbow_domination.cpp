#include "tree.h"
#include <vector>


// My implementation of the algorithm described in Figure 1 of the article below
// https://www.sciencedirect.com/science/article/pii/S0166218X09003370

int solveKRainbowDominationNumber(Tree& g, int k) {
	int n = g.edgeList.size() + 1;
	int r = 0; // final k-rainbow number
	
	// g.parentArray is the BFS ordering v1,v2,...,vn for the tree rooted at v1

	// At the beggining each vertex does not have a demanding child
	std::vector<int> demandingChildrenCount(n,0); // 's' in the article

	std::vector<int> assignedColorsCount(n,0); // 'a' in the article
	std::vector<int> requiredColorsCount(n,k); // 'b' in the article


	for (int j = n-1; j > 0; j--) {
		int currentlyDemandingChildren = demandingChildrenCount[j];

		if ( currentlyDemandingChildren > 0) {
			// Apply Theorem 4
			// int optimalSplitIndex = Theroem4();
			// r += optimalSplitIndex;
			// assignedColorsCount[j] = requiredColorsCount[optimalSplitIndex];
			// requiredColorsCount[j] = std::max(0, requiredColorsCount[j] - optimalSplitIndex);
		}
		if ( j > 0 ) {
			// Apply Theroem 3
			int parentIndex = g.parentArray[j];
			if ( assignedColorsCount[j] > 0 ) {
				requiredColorsCount[parentIndex] = 
					std::max(0, requiredColorsCount[parentIndex] - assignedColorsCount[j]);
				r += assignedColorsCount[j];
			}
			else if ( requiredColorsCount[j] > assignedColorsCount[parentIndex] ) 
				demandingChildrenCount[parentIndex]++;
		}
	}
	if ( assignedColorsCount[0] > 0 )
		r += assignedColorsCount[0];
	else if (requiredColorsCount[0] > 0)
		r++;

	return r;
}
