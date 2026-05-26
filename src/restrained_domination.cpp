#include "tree.h"
#include <vector>
#include <algorithm>
#include <limits>

const int INF = std::numeric_limits<int>::max();

struct Cost {
	int value;

	Cost(int v = 0) : value(v) {}

	bool is_impossible() const {
		return value == INF;
	}

	static Cost impossible() {
		return Cost(INF);
	}
};

Cost operator+(const Cost& a, const Cost& b) {
	if (a.is_impossible() || b.is_impossible()) {
		return Cost::impossible();
	}
	return Cost(a.value + b.value);
}

Cost& operator+=(Cost& a, const Cost& b) {
	a = a + b;
	return a;
}

bool operator<(const Cost& a, const Cost& b) {
	return a.value < b.value;
}


// Subgraph classes for DP

// 0-indexed version of 
// https://goddard.people.clemson.edu/papers/treesTablesAlgorithms.pdf
// pages 5-6

#define UNDOMINATED_UNRESTRAINED 0
#define DOMINATING 1
#define DOMINATED_UNRESTRAINED 2
#define UNDOMINATED_RESTRAINED 3
#define DOMINATED_RESTRAINED 4

std::vector<int> solveRestrainedDomination(Tree& g) {
	int n = g.edgeList.size()+1;

	std::vector<Cost> statesCosts(5);
	statesCosts[UNDOMINATED_UNRESTRAINED] = Cost(0); 
	statesCosts[DOMINATING] = Cost(1); //proper state
	statesCosts[DOMINATED_UNRESTRAINED] = Cost::impossible();
	statesCosts[UNDOMINATED_RESTRAINED] = Cost::impossible();
	statesCosts[DOMINATED_RESTRAINED] = Cost::impossible(); //proper state

	std::vector<std::vector<Cost>> dp(n, statesCosts);

	for(int i = n-1; i > 0; i--) {
		int parent = g.parentArray[i];
		
		// if parent is going to be DOMINATING 
		// there is no other restrictions we choose the cheapest cost from valid child states
		dp[parent][DOMINATING] += std::min({
			dp[i][DOMINATING],
			dp[i][UNDOMINATED_RESTRAINED],
			dp[i][DOMINATED_RESTRAINED]
		});

		// if parent is going to be DOMINATED_UNRESTRAINED 
		// then child must be dominating otherwise parent would become restrained
		Cost dominatedUnrestrainedCost = std::min(
			dp[parent][UNDOMINATED_UNRESTRAINED] + dp[i][DOMINATING],
			dp[parent][DOMINATED_UNRESTRAINED] + dp[i][DOMINATING]
		);

		// its easier to understand by seeing doc on line 42
		Cost undominatedRestrainedCost = std::min({
			dp[parent][UNDOMINATED_UNRESTRAINED] + dp[i][DOMINATED_UNRESTRAINED],
			dp[parent][UNDOMINATED_UNRESTRAINED] + dp[i][DOMINATED_RESTRAINED],
			dp[parent][UNDOMINATED_RESTRAINED] + dp[i][DOMINATED_UNRESTRAINED],
			dp[parent][UNDOMINATED_RESTRAINED] + dp[i][DOMINATED_RESTRAINED]
		});

		// its easier to understand by seeing doc on line 42
		Cost dominatedRestrainedCost = std::min({
			dp[parent][DOMINATED_UNRESTRAINED] + dp[i][DOMINATED_UNRESTRAINED],
			dp[parent][DOMINATED_UNRESTRAINED] + dp[i][DOMINATED_RESTRAINED],
			dp[parent][UNDOMINATED_RESTRAINED] + dp[i][DOMINATING],
			dp[parent][DOMINATED_RESTRAINED] + dp[i][DOMINATING],
			dp[parent][DOMINATED_RESTRAINED] + dp[i][DOMINATED_UNRESTRAINED],
			dp[parent][DOMINATED_RESTRAINED] + dp[i][DOMINATED_RESTRAINED],
		});

		dp[parent][UNDOMINATED_UNRESTRAINED] = Cost::impossible();
		dp[parent][DOMINATED_UNRESTRAINED] = dominatedUnrestrainedCost;
		dp[parent][UNDOMINATED_RESTRAINED] = undominatedRestrainedCost;
		dp[parent][DOMINATED_RESTRAINED] = dominatedRestrainedCost;
	}

	Cost minCost = std::min(dp[0][DOMINATING], dp[0][DOMINATED_RESTRAINED]);
	
	return std::vector<int>();
}
