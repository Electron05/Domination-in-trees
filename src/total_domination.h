#pragma once

#include "tree.h"
#include <vector>

enum class VertexLabel : char {
	Bound,
	Free,
	Required,
	Needed
};

std::vector<int> solveTotalDomination(Tree& g);
bool isTotalDominating(Tree& g, std::vector<int>& dominatingSet);
