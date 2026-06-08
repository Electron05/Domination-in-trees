#include "tree.h"
#include <vector>
#include <algorithm>
#include <utility>

#include "k-rainbow_number.cpp"

std::vector<BitMask> solveKRainbowDomination(Tree& g, int k) {
	int n = g.edgeList.size() + 1;
	
	std::pair<int, std::vector<int>> result = solveKRainbowDominationNumber(g, k);
	std::vector<int>& assignedColorsCount = result.second;

	std::vector<std::vector<int>> children(n);
	for (int i = 1; i < n; ++i) {
		children[g.parentArray[i]].push_back(i);
	}

	std::vector<BitMask> colorMask(n, BitMask(k));

	int rootAssigned = assignedColorsCount[0];
	if (rootAssigned > 0) {
		for (int c = 0; c < rootAssigned; ++c) {
			colorMask[0].set(c);
		}
	}

	for (int i = 0; i < n; i++) {
		if (assignedColorsCount[i] == 0) {
			// Node i has 0 colors, so its parent and children must cover all k colors.
			BitMask parentMask(k);
			if (i != 0) {
				parentMask = colorMask[g.parentArray[i]];
			}

			// Which colors the parent did not provide
			std::vector<int> missingColors;
			for (int c = 0; c < k; c++) {
				if (!parentMask.test(c)) {
					missingColors.push_back(c);
				}
			}

			// Assign missing colors to demanding children
			int missingId = 0;
			for (int j : children[i]) {
				int req = assignedColorsCount[j];
				while (req > 0) {
					if (missingId < missingColors.size()) {
						colorMask[j].set(missingColors[missingId]);
						missingId++;
						req--;
						continue;
					}
					// If all missing colors are handled, assign (any) unused colors
					for (int c = 0; c < k; c++) {
						if (!colorMask[j].test(c)) {
							colorMask[j].set(c);
							req--;
							if (req == 0) break;
						}
					}

				}
			}
		} else {
			// Node i is already assigned colors, so it does not demand domination from its children.
			// For each child j we assign colors starting from 1
			for (int j : children[i]) {
				int req = assignedColorsCount[j];
				for (int i = 0; i < req; i++) {
					colorMask[j].set(i);
				}
			}
		}
	}

	std::vector<BitMask> originalColorMask(n, BitMask(k));
	for (int i = 0; i < n; i++) {
		originalColorMask[g.originalIndices[i]] = colorMask[i];
	}

	return originalColorMask;
}

bool isKRainbowDominating(Tree& g, int k, const std::vector<BitMask>& originalColorMask) {
	int n = g.neighbourList.size();
	for (int i = 0; i < n; i++) {
		if (originalColorMask[i].count() > 0) continue;
		
		BitMask unionMask(k);
		for (int neighbour : g.neighbourList[i]) {
			unionMask |= originalColorMask[neighbour];
		}
		if (!isFull(unionMask)) {
			return false;
		}
	}
	return true;
}
