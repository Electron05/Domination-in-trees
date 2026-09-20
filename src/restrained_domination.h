#pragma once

#include "tree.h"
#include <vector>

enum NodeStateRestrained {
	UNDOMINATED_UNRESTRAINED = 0,
	DOMINATING = 1,
	DOMINATED_UNRESTRAINED = 2,
	UNDOMINATED_RESTRAINED = 3,
	DOMINATED_RESTRAINED = 4,
	STATE_COUNT = 5
};

std::vector<int> solveRestrainedDomination(Tree& g);
bool isRestrainedDominating(Tree& g, std::vector<int>& dominatingSet);
