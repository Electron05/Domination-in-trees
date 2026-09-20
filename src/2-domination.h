#pragma once

#include "tree.h"
#include <vector>

enum NodeState2Dom {
	UNDOMINATED = 0,
	DOMINATING_2DOM = 1,
	DOMINATED_ONCE = 2,
	FULLY_DOMINATED = 3,
	STATE_COUNT_2DOM = 4
};

std::vector<int> solve2Domination(Tree& g);
bool is2Dominating(Tree& g, std::vector<int>& dominatingSet);
