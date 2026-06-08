#include "tree.h"
#include <vector>
#include <algorithm>
#include <utility>

// My implementation of the algorithm described in Figure 1 of the article below
// https://www.sciencedirect.com/science/article/pii/S0166218X09003370

// Theorem 4
// It finds split index "i" for which val is minimum:
// val = requiredColorsCount[x_i] + i - 1 
// [x_1, x_2, ..., x_n] are the active children of the currentVertex, sorted by requiredColorsCount descending
// i children will be assigned 1 color each, the rest will be dominated by the parent

class RequirementsComparator {
	private:
		std::vector<int>& requiredColorsCount;
	public:
		RequirementsComparator(std::vector<int>& req) : requiredColorsCount(req) {}
		bool operator()(int firstIndex, int secondIndex) {
			return this->requiredColorsCount[firstIndex] > this->requiredColorsCount[secondIndex];
		}
};

std::pair<int, int> FindOptimalSplitIndex(Tree& g, int currentVertex, 
						  std::vector<int>& requiredColorsCount, 
						  std::vector<int>& assignedColorsCount) {
	int originalIndex = g.originalIndices[currentVertex];

	int originalParentIndex = -1;
	if (currentVertex != 0) {
		int parentIndex = g.parentArray[currentVertex];
		originalParentIndex = g.originalIndices[parentIndex];
	}

	std::vector<int> childrenIndices;
	for (int neighbour : g.neighbourList[originalIndex]) {
		// Skip the parent node
		if (currentVertex != 0 && neighbour == originalParentIndex) continue;

		int childBFSIndex = g.parentArrayIndices[neighbour];
		
		// Only collect children that are demanding
		if (assignedColorsCount[childBFSIndex] > 0) continue;
		if (requiredColorsCount[childBFSIndex] == 0) continue;
		
		childrenIndices.push_back(childBFSIndex);
	}

	// sort childrenIndices by requiredColorsCount[childrenIndex] descending
	RequirementsComparator cmp(requiredColorsCount);
	std::sort(childrenIndices.begin(), childrenIndices.end(), cmp);

	int minimumSplitValue = (int)0x7FFFFFF;
	int minimumSplitIndex = -1;
	int s = (int)childrenIndices.size();
	for (int split = 0; split <= s; split++) {
		int val;
		if (split == s) val = split;
		else val = requiredColorsCount[childrenIndices[split]] + split;
		if(val < minimumSplitValue) {
			minimumSplitValue = val;
			minimumSplitIndex = split;
		} 
	}

	int assignedCount = 0;
	if (minimumSplitIndex < s) {
		assignedCount = requiredColorsCount[childrenIndices[minimumSplitIndex]];
	}

	for (int i = 0; i < minimumSplitIndex; ++i) {
		assignedColorsCount[childrenIndices[i]] = 1;
	}

	return std::make_pair(minimumSplitIndex, assignedCount);
}

std::pair<int, std::vector<int>> solveKRainbowDominationNumber(Tree& g, int k) {
	int n = g.edgeList.size() + 1;
	int r = 0; // final k-rainbow number
	
	// g.parentArray is the BFS ordering [v_1,v_2,...,v_n] for the tree rooted at v1

	// At the beginning each vertex does not have a demanding child
	std::vector<int> demandingChildrenCount(n,0); // 's' in the article

	std::vector<int> assignedColorsCount(n,0); // 'a' in the article
	std::vector<int> requiredColorsCount(n,k); // 'b' in the article


	for (int j = n-1; j >= 0; j--) {
		int currentlyDemandingChildren = demandingChildrenCount[j];
		if (currentlyDemandingChildren > 0) {
			// Apply Theorem 4
			auto splitResult = FindOptimalSplitIndex(g, j, requiredColorsCount, assignedColorsCount);
			int optimalSplitIndex = splitResult.first;
			int assignedCount = splitResult.second;
			
			r += optimalSplitIndex;
			assignedColorsCount[j] = assignedCount;
			requiredColorsCount[j] = std::max(0, requiredColorsCount[j] - optimalSplitIndex);
		}
		if (j > 0) {
			// Apply Theorem 3
			int parentIndex = g.parentArray[j];
			if (assignedColorsCount[j] > 0) {
				requiredColorsCount[parentIndex] = 
					std::max(0, requiredColorsCount[parentIndex] - assignedColorsCount[j]);
				r += assignedColorsCount[j];
			}
			else if (requiredColorsCount[j] > assignedColorsCount[parentIndex]) 
				demandingChildrenCount[parentIndex]++;
		}
	}
	
	if (assignedColorsCount[0] > 0)
		r += assignedColorsCount[0];
	else if (requiredColorsCount[0] > 0) {
		assignedColorsCount[0] = 1;
		r++;
	}

	return std::make_pair(r, assignedColorsCount);
}
