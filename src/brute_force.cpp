#include "brute_force.h"
#include "variants.h"
#include "utils.h"
#include "tree.h"
#include <vector>
#include <string>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <bit>
#include <tuple>
#include "bitmask.h"
#include "total_domination.h"
#include "restrained_domination.h"
#include "2-domination.h"
#include "roman_domination.h"
#include "k-rainbow_domination.h"

// Exhaustive search over every labeling f: V -> {0, 1, ..., valueCount - 1}, independent of the solvers.
// Label meaning per variant:
//  - total / restrained / 2-domination: 0 = not in set, 1 = in set
//  - roman: weight 0, 1 or 2
//  - k-rainbow: bitmask of assigned colors
//
// Vertices are labeled in parentArray (BFS) order. Children always have bigger indices than their parent,
// so after labeling the last child of v, the whole closed neighbourhood of v is known and v can be checked.
// Branches are cut as soon as a checked vertex is not dominated or the weight is not better than the best found.

namespace {
struct SearchState;
using IsDominated = bool (*)(int v, const SearchState& s);

struct SearchState {
	int n;
	std::vector<std::vector<int>> neighbours; // in parentArray indices
	std::vector<std::vector<int>> checkAfter; // vertices which can be checked right after labeling vertex i
	std::vector<int> values;                  // allowed labels, sorted by weight ascending
	std::vector<int> weights;                 // weights[label]
	std::vector<int> labels;
	VariantType type;
	IsDominated isDominated;
	int best;

	// Used only by K_RAINBOW
	int k;
	std::vector<BitMask> colorMasks;          // labels as BitMask (kept in sync with labels)
};

// ====================================================
// Domination conditions, checked for vertex v once its whole closed neighbourhood is labeled.
// Same per-vertex checks as the solvers' verifiers use.
// ====================================================
bool isTotalDominated(int v, const SearchState& s) {
	return isVertexTotallyDominated(s.neighbours[v], s.labels);
}

bool isRestrainedDominated(int v, const SearchState& s) {
	return isVertexRestrainedDominated(v, s.neighbours[v], s.labels);
}

bool is2Dominated(int v, const SearchState& s) {
	return isVertex2Dominated(v, s.neighbours[v], s.labels);
}

bool isRomanDefended(int v, const SearchState& s) {
	return isVertexRomanDefended(v, s.neighbours[v], s.labels);
}

bool isKRainbowDominated(int v, const SearchState& s) {
	return isVertexKRainbowDominated(v, s.k, s.neighbours[v], s.colorMasks);
}

class WeightComparator {
	private:
		const std::vector<int>& weights;
	public:
		WeightComparator(const std::vector<int>& w) : weights(w) {}
		bool operator()(int firstLabel, int secondLabel) const {
			return this->weights[firstLabel] < this->weights[secondLabel];
		}
};

// K_RAINBOW only: colors are interchangeable, so we only look at labelings where new colors appear in order 0, 1, 2, ...
// Label is allowed if its bits above already used colors form a block starting at usedColors
bool isCanonicalColoring(int label, int usedColors) {
	unsigned newColors = (unsigned)label >> usedColors;
	return (newColors & (newColors + 1)) == 0;
}

// ====================================================
// search() - tries labels for vertex i, then calls itself for vertex i + 1
//
// Labels for vertices 0..i-1 are already chosen and they cost "weight" in total.
// Now we try every label for vertex i, from the cheapest one:
//
//  1. Price check. If weight + price of this label is not smaller than the best solution
//     found so far, stop. Labels are sorted by price, so all the next ones are too expensive as well.
//  2. (K_RAINBOW only) Skip the label if it only renames colors of a labeling we already tried.
//     E.g. {1} on vertex 0 and {2} on vertex 0 lead to the same answers, so only {1} is tried.
//     usedColors = how many colors vertices 0..i-1 use (always 0 for other variants).
//  3. Write the label into s.labels[i].
//  4. Check every vertex from checkAfter[i]. Their whole neighbourhood is labeled now,
//     so their condition can not change anymore. If one of them is not dominated,
//     no choice for vertices i+1..n-1 can fix it, so this label is skipped.
//  5. Otherwise go deeper: search(i + 1).
//
// When i == n every vertex has a label and every vertex passed its check,
// so this is a valid solution cheaper than s.best (step 1 guarantees that) and it becomes the new best.
//
// Example: total domination on path 2 - 0 - 1 - 3 - 4, rooted at 0
//
//        0
//       / \
//      1   2          parentArray = [-1, 0, 0, 1, 3]
//      |              checkAfter  = [ [], [], [0, 2], [1], [3, 4] ]
//      3                after labeling 2 -> check 0 and 2
//      |                after labeling 3 -> check 1
//      4                after labeling 4 -> check 3 and 4
//
// Notation:
//   [1 0 1 _ _]  labels of vertices 0, 1, 2, 3, 4   (1 = in set, 0 = not in set, _ = no label yet)
//   w            weight so far (how many vertices are in set)
// best starts at 11 (2V + 1, no solution known yet).
// Steps where nothing can be checked yet are left out.
//
//   [0 0 0 _ _]  w=0  check 0: neighbours 1, 2 not in set                -> skip
//   [0 0 1 _ _]  w=1  check 0: OK, check 2: neighbour 0 not in set      -> skip
//   [0 1 0 _ _]  w=1  check 0: OK, check 2: neighbour 0 not in set      -> skip
//   [0 1 1 _ _]  w=2  check 0: OK, check 2: neighbour 0 not in set      -> skip
//   [1 0 0 _ _]  w=1  check 0: neighbours 1, 2 not in set                -> skip
//   [1 0 1 _ _]  w=2  check 0, 2: OK
//   [1 0 1 0 _]  w=2  check 1: OK
//   [1 0 1 0 0]  w=2  check 3: neighbours 1, 4 not in set                -> skip
//   [1 0 1 0 1]  w=3  check 3: OK, check 4: neighbour 3 not in set      -> skip
//   [1 0 1 1 _]  w=3  check 1: OK
//   [1 0 1 1 0]  w=3  check 3: neighbours 1, 4 not in set                -> skip
//   [1 0 1 1 1]  w=4  check 3, 4: OK, all labeled                        -> best = 4  (first solution)
//   [1 1 0 _ _]  w=2  check 0, 2: OK
//   [1 1 0 0 _]  w=2  check 1: OK
//   [1 1 0 0 0]  w=2  check 3: OK, check 4: neighbour 3 not in set      -> skip
//   [1 1 0 0 1]  w=3  check 3: OK, check 4: neighbour 3 not in set      -> skip
//   [1 1 0 1 _]  w=3  check 1: OK
//   [1 1 0 1 0]  w=3  check 3, 4: OK, all labeled                        -> best = 3  (improved)
//   [1 1 0 1 1]  w=4  4 is not smaller than best 3                       -> stop
//   [1 1 1 _ _]  w=3  3 is not smaller than best 3                       -> stop
// Result: 3 (vertices 0, 1, 3)
// ====================================================
void search(SearchState& s, int i, int weight, int usedColors) {
	if (i == s.n) {
		s.best = weight;
		return;
	}

	for (int label : s.values) {
		// 1.
		int newWeight = weight + s.weights[label];
		if (newWeight >= s.best) break;

		// 2.
		int newUsedColors = usedColors;
		if (s.type == VariantType::K_RAINBOW) {
			if (!isCanonicalColoring(label, usedColors)) continue;
			newUsedColors += std::popcount((unsigned)label >> usedColors);
		}

		// 3.
		s.labels[i] = label;
		if (s.type == VariantType::K_RAINBOW) {
			s.colorMasks[i] = BitMask((unsigned long long)label);
		}

		// 4.
		bool dominated = true;
		for (int v : s.checkAfter[i]) {
			if (!s.isDominated(v, s)) {
				dominated = false;
				break;
			}
		}
		// 5.
		if (dominated) {
			search(s, i + 1, newWeight, newUsedColors);
		}
	}
}

// ====================================================
// runSearch() - prepares everything search() needs, runs it and returns the smallest weight
//
// Every vertex gets one label from 0..valueCount-1, every label has a price (weights[label]).
// We look for the cheapest labeling in which every vertex is dominated.
// Vertices are labeled one by one in parentArray order (vertex 0 first, then 1, 2, ...).
//
// 1. Neighbours
//    Built from parentArray, so vertex numbers match the labeling order.
//
// 2. checkAfter - when can a vertex be checked?
//    Whether vertex v is dominated depends on the labels of v and all its neighbours.
//    So v can be checked only when all of them are labeled.
//    In parentArray a parent always comes before its children, so the last of them to get
//    a label is the last child of v (or v itself if it is a leaf).
//    checkAfter[i] = vertices which can be checked right after vertex i gets its label.
//
//    Example: parentArray [-1, 0, 0, 1]   (0 has children 1 and 2, 1 has child 3)
//      vertex 0 can be checked after labeling 2 (its last child)
//      vertex 1 can be checked after labeling 3 (its only child)
//      vertex 2 can be checked after labeling 2 (leaf, parent 0 is already labeled)
//      vertex 3 can be checked after labeling 3 (leaf)
//      checkAfter = [ [], [], [0, 2], [1, 3] ]
//        after labeling 2 -> check 0 and 2
//        after labeling 3 -> check 1 and 3
//
// 3. Labels sorted by price
//    E.g. Roman: 0, 1, 2. K-rainbow: {}, then single colors, then pairs, ...
//    search() tries them in this order, so the first too expensive label ends the loop.
//
// 4. Starting limit
//    upperBound is the weight of a solution we already have (the algorithm's answer),
//    so only strictly cheaper labelings are searched. Each time search() finds one,
//    the limit drops to its weight. Result is the optimum, or upperBound if nothing cheaper exists.
// ====================================================
int runSearch(Tree& g, int upperBound, const VariantInfo& variant, int valueCount, std::vector<int> weights, IsDominated isDominated) {
	SearchState s;
	s.n = g.neighbourList.size();
	s.neighbours.assign(s.n, std::vector<int>());
	s.checkAfter.assign(s.n, std::vector<int>());
	s.labels.assign(s.n, 0);
	s.weights = std::move(weights);
	s.type = variant.type;
	s.isDominated = isDominated;
	s.k = variant.k;
	s.colorMasks.assign(s.n, BitMask());

	// 1. and 2.
	std::vector<int> lastLabeled(s.n);
	for (int v = 0; v < s.n; v++) {
		lastLabeled[v] = v;
	}
	for (int i = 1; i < s.n; i++) {
		int parent = g.parentArray[i];
		s.neighbours[i].push_back(parent);
		s.neighbours[parent].push_back(i);
		lastLabeled[parent] = std::max(lastLabeled[parent], i);
	}
	for (int v = 0; v < s.n; v++) {
		s.checkAfter[lastLabeled[v]].push_back(v);
	}

	// 3.
	for (int label = 0; label < valueCount; label++) {
		s.values.push_back(label);
	}
	WeightComparator cmp(s.weights);
	std::stable_sort(s.values.begin(), s.values.end(), cmp);

	// 4.
	s.best = upperBound;
	search(s, 0, 0, 0);
	return s.best;
}

std::string edgeListToString(const Tree& g) {
	std::stringstream ss;
	for (const auto& edge : g.edgeList) {
		ss << std::get<0>(edge) << "-" << std::get<1>(edge) << " ";
	}
	return ss.str();
}

struct EdgeCase {
	int k;
	std::vector<std::tuple<int, int>> edges;
};

// Trees on which the k-rainbow algorithm returned non-optimal weight (found by the brute force tests)
const std::vector<EdgeCase> kRainbowEdgeCases = {
	{3, {{0, 1}, {0, 5}, {1, 2}, {2, 3}, {2, 4}, {5, 6}, {6, 7}}},                 // algorithm = 7, optimum = 6
	{5, {{0, 1}, {0, 6}, {0, 8}, {0, 9}, {1, 2}, {2, 3}, {2, 4}, {2, 5}, {6, 7}}}  // algorithm = 10, optimum = 9
};

// Returns true if the algorithm's solution is valid and has optimal weight, otherwise fills failure description
bool testTree(const VariantInfo& variant, Tree& tree, std::string& failure) {
	SolutionCheck algorithm = checkSolution(variant, tree);
	// Valid algorithm solution is an upper bound, so brute force only looks for something strictly better.
	// Every variant has a solution of weight at most 2V, so invalid solutions fall back to 2V + 1
	int fallbackBound = 2 * (int)tree.neighbourList.size() + 1;
	int upperBound = algorithm.valid ? algorithm.weight : fallbackBound;
	int optimum = bruteForceMinimumWeight(variant, tree, upperBound);
	if (algorithm.valid && algorithm.weight == optimum) return true;

	std::stringstream ss;
	ss << "V = " << tree.neighbourList.size()
	   << ", algorithm = " << algorithm.weight << (algorithm.valid ? "" : " (invalid)")
	   << ", brute force = " << optimum
	   << ", edges: " << edgeListToString(tree);
	failure = ss.str();
	return false;
}
}

// upperBound has to be the weight of some valid solution (or anything bigger),
// the result is min(upperBound, optimum)
int bruteForceMinimumWeight(const VariantInfo& variant, Tree& g, int upperBound) {
	switch (variant.type) {
		case VariantType::TOTAL:
			return runSearch(g, upperBound, variant, 2, {0, 1}, isTotalDominated);
		case VariantType::RESTRAINED:
			return runSearch(g, upperBound, variant, 2, {0, 1}, isRestrainedDominated);
		case VariantType::TWO_DOMINATION:
			return runSearch(g, upperBound, variant, 2, {0, 1}, is2Dominated);
		case VariantType::ROMAN:
			return runSearch(g, upperBound, variant, 3, {0, 1, 2}, isRomanDefended);
		case VariantType::K_RAINBOW: {
			int valueCount = 1 << variant.k;
			std::vector<int> weights(valueCount);
			for (int label = 0; label < valueCount; label++) {
				weights[label] = std::popcount((unsigned)label);
			}
			return runSearch(g, upperBound, variant, valueCount, weights, isKRainbowDominated);
		}
	}
	return -1;
}

void runBruteForceTests(const std::string& outputFolder) {
	int maxSize = parseInt(std::getenv("BRUTE_FORCE_MAX_SIZE"));
	if (maxSize <= 0) return;

	std::vector<int> kValues = parseIntegerList(std::getenv("K_RAINBOW_VALUES"));

	std::string filepath = "src/input/generated_trees/brute_force_trees.txt";
	std::ifstream file(filepath);
	if (!file.is_open()) {
		std::cerr << "Error: Could not open file " << filepath << std::endl;
		return;
	}

	std::vector<Tree> trees;
	openAllTrees(file, trees);

	std::ofstream logFile(outputFolder + "/brute_force_log.txt");

	writeLog(logFile, "\n==============================");
	writeLog(logFile, "Brute force tests: all trees with V = 2.." + std::to_string(maxSize) + " (" + std::to_string(trees.size()) + " trees)");
	writeLog(logFile, "==============================");

	std::vector<VariantInfo> variants = createVariants(kValues);

	for (const auto& variant : variants) {
		writeLog(logFile, " - " + variant.name + ":");

		int failedCount = 0;
		std::string firstFailure;

		// Trees in the file are ordered by size, progress is logged after each size
		size_t t = 0;
		while (t < trees.size()) {
			size_t size = trees[t].neighbourList.size();
			int sizeTreeCount = 0;
			int sizeFailedCount = 0;
			auto t0 = std::chrono::steady_clock::now();

			while (t < trees.size() && trees[t].neighbourList.size() == size) {
				std::string failure;
				if (!testTree(variant, trees[t], failure)) {
					sizeFailedCount++;
					if (firstFailure.empty()) firstFailure = failure;
				}
				sizeTreeCount++;
				t++;
			}

			auto t1 = std::chrono::steady_clock::now();
			auto testTime = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
			failedCount += sizeFailedCount;

			std::string result = (sizeFailedCount == 0)
				? "OK"
				: "FAILED (" + std::to_string(sizeFailedCount) + ")";
			writeLog(logFile, "     " + padRight("V = " + std::to_string(size) + ":", 9)
			                  + padLeft(std::to_string(sizeTreeCount), 4) + " trees  "
			                  + padRight(result, 12) + " | " + padLeft(std::to_string(testTime) + " ms", 9));
		}

		if (failedCount == 0) {
			writeLog(logFile, "     all OK");
		} else {
			writeLog(logFile, "     FAILED " + std::to_string(failedCount) + "/" + std::to_string(trees.size()) + " trees, first failure: " + firstFailure);
		}
	}

	writeLog(logFile, "\n==============================");
	writeLog(logFile, "Hardcoded edge cases (k-rainbow counterexamples)");
	writeLog(logFile, "==============================");

	for (const auto& edgeCase : kRainbowEdgeCases) {
		VariantInfo variant = {std::to_string(edgeCase.k) + "-Rainbow domination", VariantType::K_RAINBOW, edgeCase.k};

		Tree tree;
		tree.edgeList = edgeCase.edges;
		edgesToParentArray(tree);

		std::string label = " - " + variant.name + ": ";
		if (edgeCase.k > MAX_K) {
			writeLog(logFile, label + "SKIPPED (k > MAX_K = " + std::to_string(MAX_K) + ")");
			continue;
		}

		std::string failure;
		if (testTree(variant, tree, failure)) {
			writeLog(logFile, label + "OK");
		} else {
			writeLog(logFile, label + "FAILED, " + failure);
		}
	}
}
