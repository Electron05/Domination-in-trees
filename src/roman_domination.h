#pragma once

#include "tree.h"
#include <vector>
#include <utility>

enum NodeStateRoman {
	UNDEFENDED = 0,
	SELF_DEFENDED = 1,
	DEFENDING = 2,
	DEFENDED = 3,
	STATE_COUNT_ROMAN = 4
};

std::pair<std::vector<int>, std::vector<int>> solveRomanDomination(Tree& g);
bool isRomanDominating(Tree& g, std::vector<int>& selfDefended, std::vector<int> defending);
