#include "tree.h"
#include <vector>
#include <algorithm>
#include <limits>
#include "DP_cost.h"

// ====================================================
// Node States definitions
// ====================================================
enum NodeState2Dom {
	UNDOMINATED = 0,
	DOMINATING_2DOM = 1,
	DOMINATED_ONCE = 2,
	FULLY_DOMINATED = 3,
	STATE_COUNT_2DOM = 4
};

// Transitions format: {Old Parent State, Child State}
// These dictate how a parent transitions into a new state when a specific child is added.
const std::vector<std::pair<int, int>> undominated2DomTransitions = {
	{UNDOMINATED, FULLY_DOMINATED}
};

const std::vector<std::pair<int, int>> dominating2DomTransitions = {
	{DOMINATING_2DOM, DOMINATING_2DOM},
	{DOMINATING_2DOM, DOMINATED_ONCE},
	{DOMINATING_2DOM, FULLY_DOMINATED}
};

const std::vector<std::pair<int, int>> dominatedOnce2DomTransitions = {
	{UNDOMINATED, DOMINATING_2DOM},
	{DOMINATED_ONCE, FULLY_DOMINATED}
};

const std::vector<std::pair<int, int>> fullyDominated2DomTransitions = {
	{DOMINATED_ONCE, DOMINATING_2DOM},
	{FULLY_DOMINATED, DOMINATING_2DOM},
	{FULLY_DOMINATED, FULLY_DOMINATED}
};

std::vector<int> solve2Domination(Tree& g) {
	int n = g.edgeList.size() + 1;

	// Initialize DP table. 
	// dp[node][state] stores the minimum cost of the subtree rooted at 'node' 
	// given that 'node' is in 'state'.
	std::vector<Cost> initialStates(STATE_COUNT_2DOM);
	initialStates[UNDOMINATED]     = Cost(0); 
	initialStates[DOMINATING_2DOM]      = Cost(1); // Node is part of dominating set
	initialStates[DOMINATED_ONCE]  = Cost::impossible();
	initialStates[FULLY_DOMINATED] = Cost::impossible();

	std::vector<std::vector<Cost>> dp(n, initialStates);

	// ====================================================
	// Backtracking vectors
	// We must track what the child's state was, and what the parent's state 
	// was before this child was added, to reconstruct the final dominating set.
	// ====================================================
	std::vector<int> stateRow(STATE_COUNT_2DOM);
	std::vector<std::vector<int>> chosenChildState(n, stateRow);
	std::vector<std::vector<int>> prevParentState(n, stateRow); 

	// Bottom-up traversal
	for(int i = n - 1; i > 0; i--) {
		int parent = g.parentArray[i];
		
		int bestCost;
		int bestChildState;
		int bestPreviousParentState;

		// ====================================================
		// STATE 0: Parent becomes UNDOMINATED_2DOM
		// ====================================================
		bestCost = INF;
		bestPreviousParentState = INF;
		bestChildState = INF;
		for(const auto& transition : undominated2DomTransitions) {
			Cost cost = dp[parent][transition.first] + dp[i][transition.second];
			if(cost < bestCost) {
				bestCost = cost.value;
				bestPreviousParentState = transition.first;
				bestChildState = transition.second;
			}
		}
		Cost undominatedCost = Cost(bestCost);
		chosenChildState[i][UNDOMINATED] = bestChildState;
		prevParentState[i][UNDOMINATED] = bestPreviousParentState;

		// ====================================================
		// STATE 1: Parent becomes DOMINATING_2DOM
		// ====================================================
		bestCost = INF;
		bestPreviousParentState = INF;
		bestChildState = INF;
		for(const auto& transition : dominating2DomTransitions) {
			Cost cost = dp[parent][transition.first] + dp[i][transition.second];
			if(cost < bestCost) {
				bestCost = cost.value;
				bestPreviousParentState = transition.first;
				bestChildState = transition.second;
			}
		}
		Cost dominatingCost = Cost(bestCost);
		chosenChildState[i][DOMINATING_2DOM] = bestChildState;
		prevParentState[i][DOMINATING_2DOM] = bestPreviousParentState;

		// ====================================================
		// STATE 2: Parent becomes DOMINATED_ONCE_2DOM
		// ====================================================
		bestCost = INF;
		bestPreviousParentState = INF;
		bestChildState = INF;
		for(const auto& transition : dominatedOnce2DomTransitions) {
			Cost cost = dp[parent][transition.first] + dp[i][transition.second];
			if(cost < bestCost) {
				bestCost = cost.value;
				bestPreviousParentState = transition.first;
				bestChildState = transition.second;
			}
		}
		Cost dominatedOnceCost = Cost(bestCost);
		chosenChildState[i][DOMINATED_ONCE] = bestChildState;
		prevParentState[i][DOMINATED_ONCE] = bestPreviousParentState;

		// ====================================================
		// STATE 3: Parent becomes FULLY_DOMINATED_2DOM
		// ====================================================
		bestCost = INF;
		bestPreviousParentState = INF;
		bestChildState = INF;
		for(const auto& transition : fullyDominated2DomTransitions) {
			Cost cost = dp[parent][transition.first] + dp[i][transition.second];
			if(cost < bestCost) {
				bestCost = cost.value;
				bestPreviousParentState = transition.first;
				bestChildState = transition.second;
			}
		}
		Cost fullyDominatedCost = Cost(bestCost);
		chosenChildState[i][FULLY_DOMINATED] = bestChildState;
		prevParentState[i][FULLY_DOMINATED] = bestPreviousParentState;

		// Commit new parent states.
		dp[parent][UNDOMINATED] = undominatedCost;
		dp[parent][DOMINATING_2DOM] = dominatingCost;
		dp[parent][DOMINATED_ONCE] = dominatedOnceCost;
		dp[parent][FULLY_DOMINATED] = fullyDominatedCost;
	}
	
	// ====================================================
	// Backtracking Phase: Reconstruct the dominating set
	// ====================================================
	std::vector<int> dominatingSet;
	std::vector<int> finalStates(n);

	// The root must end up either DOMINATING or properly FULLY_DOMINATED
	int finalRootState = (dp[0][DOMINATING_2DOM] < dp[0][FULLY_DOMINATED]) ? DOMINATING_2DOM : FULLY_DOMINATED;	 
	finalStates[0] = finalRootState;
	
	if(finalRootState == DOMINATING_2DOM) dominatingSet.push_back(g.originalIndices[0]);

	// Traverse top-down to replay the state history
	for (int i = 1; i < n; i++) {
		int parent = g.parentArray[i];
		int currentParentState = finalStates[parent];
		
		// Determine what state this child had to be in
		finalStates[i] = chosenChildState[i][currentParentState];
		if(finalStates[i] == DOMINATING_2DOM) {
			dominatingSet.push_back(g.originalIndices[i]);
		}

		// Roll back the parent's state to what it was before this child was added
		finalStates[parent] = prevParentState[i][currentParentState];
	}

	return dominatingSet;
}

bool is2Dominating(Tree& g, std::vector<int>& dominatingSet) {
	int n = g.neighbourList.size();
	std::vector<bool> inSet(n, false);
	for (int u : dominatingSet) {
		inSet[u] = true;
	}

	for (int i = 0; i < n; i++) {
		if (inSet[i]) continue;
		int dominatingNeighbours = 0;

		for (int neighbour : g.neighbourList[i]) {
			if (inSet[neighbour]) {
				dominatingNeighbours++;
			}
		}

		if (dominatingNeighbours < 2) {
			return false;
		}
	}
	return true;
}
