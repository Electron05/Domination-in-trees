#pragma once

#include "tree.h"
#include <vector>
#include <string>

enum class VariantType : char {
	TOTAL,
	RESTRAINED,
	TWO_DOMINATION,
	ROMAN,
	K_RAINBOW
};

struct VariantInfo {
	std::string name;
	VariantType type;
	int k = 0;
};

struct SolutionCheck {
	bool valid;
	int weight; // size of the dominating set / sum of the dominating function
};

std::vector<VariantInfo> createVariants(const std::vector<int>& kValues);
SolutionCheck checkSolution(const VariantInfo& variant, Tree& tree);
