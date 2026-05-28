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


// Transitions for parent to become UNDOMINATED_RESTRAINED (State 3)
// Format: {Old Parent State, Child State}
// Its easier to understand by seeing doc on line 42
const std::vector<std::pair<int, int>> undominatedRestrainedTransitions = {
    {UNDOMINATED_UNRESTRAINED, DOMINATED_UNRESTRAINED},
    {UNDOMINATED_UNRESTRAINED, DOMINATED_RESTRAINED},
    {UNDOMINATED_RESTRAINED, DOMINATED_UNRESTRAINED},
    {UNDOMINATED_RESTRAINED, DOMINATED_RESTRAINED}
};

// Transitions for parent to become DOMINATED_RESTRAINED (State 4)
// Format: {Old Parent State, Child State}
// Its easier to understand by seeing doc on line 42
const std::vector<std::pair<int, int>> dominatedRestrainedTransitions = {
    {DOMINATED_UNRESTRAINED, DOMINATED_UNRESTRAINED},
    {DOMINATED_UNRESTRAINED, DOMINATED_RESTRAINED},
    {UNDOMINATED_RESTRAINED, DOMINATING},
    {DOMINATED_RESTRAINED, DOMINATING},
    {DOMINATED_RESTRAINED, DOMINATED_UNRESTRAINED},
    {DOMINATED_RESTRAINED, DOMINATED_RESTRAINED}
};



std::vector<int> solveRestrainedDomination(Tree& g) {
	int n = g.edgeList.size()+1;

	std::vector<Cost> statesCosts(5);
	statesCosts[UNDOMINATED_UNRESTRAINED] = Cost(0); 
	statesCosts[DOMINATING] = Cost(1); //proper state
	statesCosts[DOMINATED_UNRESTRAINED] = Cost::impossible();
	statesCosts[UNDOMINATED_RESTRAINED] = Cost::impossible();
	statesCosts[DOMINATED_RESTRAINED] = Cost::impossible(); //proper state

	std::vector<std::vector<Cost>> dp(n, statesCosts);

	// ====== ======= Backtracking vectors ================
	
	std::vector<int> stateRow(5);
	std::vector<std::vector<int>> chosenChildState(n,stateRow);
	// child i must be in chosenChildState[i][j] so their parent can be in state j

	std::vector<std::vector<int>> prevParentState(n,stateRow); 
	// prevParentState[i][DOMINATED_RESTRAINED] = DOMINATED_UNRESTRAINED / UNDOMINATED_RESTRAINED / DOMINATED_RESTRAINED
	// after adding child i to its parent, parent became DOMINATED_RESTRAINED
	// value of this cell determines prevoius state of the parent
	// its essential to correctly read chosenChildState[i][curentParentState];

	// ====================================================

	for(int i = n-1; i > 0; i--) {
		int parent = g.parentArray[i];
		
		std::vector<int> possibleParentStates;
		std::vector<int> possibleChildStates;

		// ==========  DOMINATING ===============
		// if parent is going to become DOMINATING, then child must be in one of the following states
		
		int bestCost = INF;
		int bestChildState = INF;
		possibleChildStates = {DOMINATING,UNDOMINATED_RESTRAINED,DOMINATED_RESTRAINED};
		for(int p : possibleChildStates){
			if(dp[i][p] < bestCost){ 
				bestCost = dp[i][p].value;
				bestChildState = p;
			}
		}
		chosenChildState[i][DOMINATING] = bestChildState;
		prevParentState[i][DOMINATING] = DOMINATING;
		dp[parent][DOMINATING] += dp[i][bestChildState];

		// ======================================


		// ==========  DOMINATED_UNRESTRAINED  ===============
		// if parent is going to be DOMINATED_UNRESTRAINED 
		// then child must be dominating otherwise parent would become restrained

		bestChildState = DOMINATING;
		int bestPreviousParentState = 
			dp[parent][UNDOMINATED_UNRESTRAINED] < dp[parent][DOMINATED_UNRESTRAINED] ? UNDOMINATED_UNRESTRAINED : DOMINATED_UNRESTRAINED;

		Cost dominatedUnrestrainedCost = std::min(
			dp[parent][UNDOMINATED_UNRESTRAINED] + dp[i][DOMINATING],
			dp[parent][DOMINATED_UNRESTRAINED] + dp[i][DOMINATING]
		);

		chosenChildState[i][DOMINATED_UNRESTRAINED] = bestChildState;
		prevParentState[i][DOMINATED_UNRESTRAINED] = bestPreviousParentState;

		// ===================================================

		// ============= UNDOMINATED_RESTRAINED ==============

		bestCost = INF;
		for(const auto& transition : undominatedRestrainedTransitions){
			Cost cost = dp[parent][transition.first] + dp[i][transition.second];
			if(cost < bestCost){
				bestCost = cost.value;
				bestPreviousParentState = transition.first;
				bestChildState = transition.second;
			}
		}
		Cost undominatedRestrainedCost = Cost(bestCost);
		chosenChildState[i][UNDOMINATED_RESTRAINED] = bestChildState;
		prevParentState[i][UNDOMINATED_RESTRAINED] = bestPreviousParentState;

		// ====================================================


		// ============= DOMINATED_RESTRAINED ==============
		// its easier to understand by seeing doc on line 42

		bestCost = INF;
		for(const auto& transition : dominatedRestrainedTransitions){
			Cost cost = dp[parent][transition.first] + dp[i][transition.second];
			if(cost < bestCost){
				bestCost = cost.value;
				bestPreviousParentState = transition.first;
				bestChildState = transition.second;
			}
		}
		Cost dominatedRestrainedCost = Cost(bestCost);
		chosenChildState[i][DOMINATED_RESTRAINED] = bestChildState;
		prevParentState[i][DOMINATED_RESTRAINED] = bestPreviousParentState;

		// ==================================================


		dp[parent][UNDOMINATED_UNRESTRAINED] = Cost::impossible();
		dp[parent][DOMINATED_UNRESTRAINED] = dominatedUnrestrainedCost;
		dp[parent][UNDOMINATED_RESTRAINED] = undominatedRestrainedCost;
		dp[parent][DOMINATED_RESTRAINED] = dominatedRestrainedCost;
	
	}
	
	std::vector<int> dominatingSet;
	std::vector<int> finalStates(n);

	int finalRootState = (dp[0][DOMINATING] < dp[0][DOMINATED_RESTRAINED]) ? DOMINATING : DOMINATED_RESTRAINED;
	finalStates[0] = finalRootState;
	
	if(finalRootState == DOMINATING) dominatingSet.push_back(g.orginalIndices[0]);

	for (int i = 1; i < n; i++) {
		int parent = g.parentArray[i];
		
		int currentParentState = finalStates[parent];
		
		finalStates[i] = chosenChildState[i][currentParentState];
		if(finalStates[i] == DOMINATING){
			dominatingSet.push_back(g.orginalIndices[i]);
		}


		finalStates[parent] = prevParentState[i][currentParentState];
	}

	return dominatingSet;
}
