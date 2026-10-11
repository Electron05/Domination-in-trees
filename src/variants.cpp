#include "variants.h"
#include "total_domination.h"
#include "restrained_domination.h"
#include "2-domination.h"
#include "roman_domination.h"
#include "k-rainbow_domination.h"
#include <vector>
#include <string>
#include <utility>

std::vector<VariantInfo> createVariants(const std::vector<int>& kValues) {
	std::vector<VariantInfo> variants = {
		{"Total domination", VariantType::TOTAL, 0},
		{"Restrained domination", VariantType::RESTRAINED, 0},
		{"2-domination", VariantType::TWO_DOMINATION, 0},
		{"Roman domination", VariantType::ROMAN, 0}
	};
	for (int k : kValues) {
		variants.push_back({
			std::to_string(k) + "-Rainbow domination",
			VariantType::K_RAINBOW,
			k
		});
	}
	return variants;
}

// Solves the tree and checks if the result is a valid dominating set / function of its variant
SolutionCheck checkSolution(const VariantInfo& variant, Tree& tree) {
	switch (variant.type) {
		case VariantType::TOTAL: {
			std::vector<int> dominatingSet = solveTotalDomination(tree);
			return {isTotalDominating(tree, dominatingSet), (int)dominatingSet.size()};
		}
		case VariantType::RESTRAINED: {
			std::vector<int> dominatingSet = solveRestrainedDomination(tree);
			return {isRestrainedDominating(tree, dominatingSet), (int)dominatingSet.size()};
		}
		case VariantType::TWO_DOMINATION: {
			std::vector<int> dominatingSet = solve2Domination(tree);
			return {is2Dominating(tree, dominatingSet), (int)dominatingSet.size()};
		}
		case VariantType::ROMAN: {
			std::pair<std::vector<int>, std::vector<int>> romanSets = solveRomanDomination(tree);
			int weight = (int)romanSets.first.size() + 2 * (int)romanSets.second.size();
			return {isRomanDominating(tree, romanSets.first, romanSets.second), weight};
		}
		case VariantType::K_RAINBOW: {
			std::vector<BitMask> colorMask = solveKRainbowDomination(tree, variant.k);
			int weight = 0;
			for (const BitMask& mask : colorMask) {
				weight += (int)mask.count();
			}
			return {isKRainbowDominating(tree, variant.k, colorMask), weight};
		}
	}
	return {false, 0};
}
