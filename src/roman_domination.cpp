#include "tree.h"
#include "DP_cost.h"
#include <vector>

// ====================================================
// Node States definitions
// ====================================================
enum NodeStateRoman {
	UNDEFENDED = 0,
	SELF_DEFENDED = 1,
	DEFENDING = 2,
	DEFENDED = 3,
	STATE_COUNT_ROMAN = 4
};

// Transitions format: {Old Parent State, Child State}
// These dictate how a parent transitions into a new state when a specific child is added.
const std::vector<std::pair<int, int>> undefendedRomanTransitions = {
	{UNDEFENDED, SELF_DEFENDED},
	{UNDEFENDED, DEFENDED}
};

const std::vector<std::pair<int, int>> selfDefendedRomanTransitions = {
	{SELF_DEFENDED, DEFENDED},
	{SELF_DEFENDED, DEFENDING},
	{SELF_DEFENDED, SELF_DEFENDED}
};

const std::vector<std::pair<int, int>> defendingRomanTransitions = {
	{DEFENDING, UNDEFENDED},
	{DEFENDING, DEFENDING},
	{DEFENDING, DEFENDED},
	{DEFENDING, SELF_DEFENDED},
};

const std::vector<std::pair<int, int>> defendedRomanTransitions = {
	{UNDEFENDED, DEFENDING},
	{DEFENDED, DEFENDING},
	{DEFENDED, DEFENDED},
	{DEFENDED, SELF_DEFENDED}
};



std::pair<std::vector<int>, std::vector<int>> solveRomanDomination(Tree& g) {
	int n = g.neighbourList.size();

	// Initialize DP table. 
	// dp[node][state] stores the minimum cost of the subtree rooted at 'node' 
	// given that 'node' is in 'state'.
	std::vector<Cost> initialStates(STATE_COUNT_ROMAN);
	initialStates[UNDEFENDED]     = Cost(0); 
	initialStates[SELF_DEFENDED]  = Cost(1);
	initialStates[DEFENDING]      = Cost(2);
	initialStates[DEFENDED]       = Cost::impossible();

	std::vector<std::vector<Cost>> dp(n, initialStates);

	// ====================================================
	// Backtracking vectors
	// We must track what the child's state was, and what the parent's state 
	// was before this child was added, to reconstruct the final dominating set.
	// ====================================================
	std::vector<int> stateRow(STATE_COUNT_ROMAN);
	std::vector<std::vector<int>> chosenChildState(n, stateRow);
	std::vector<std::vector<int>> prevParentState(n, stateRow); 

	// Bottom-up traversal
	for(int i = n - 1; i > 0; i--) {
		int parent = g.parentArray[i];
		
		int bestCost;
		int bestChildState;
		int bestPreviousParentState;

		// ====================================================
		// STATE 0: Parent becomes UNDEFENDED
		// ====================================================
		bestCost = INF;
		bestChildState = INF;
		for(const auto& transition : undefendedRomanTransitions) {
			Cost cost = dp[parent][transition.first] + dp[i][transition.second];
			if(cost < bestCost) {
				bestCost = cost.value;
				bestChildState = transition.second;
			}
		}
		Cost undefendedCost = Cost(bestCost);
		chosenChildState[i][UNDEFENDED] = bestChildState;
		prevParentState[i][UNDEFENDED] = UNDEFENDED;

		// ====================================================
		// STATE 1: Parent becomes SELF_DEFENDED
		// ====================================================
		bestCost = INF;
		bestChildState = INF;
		for(const auto& transition : selfDefendedRomanTransitions) {
			Cost cost = dp[parent][transition.first] + dp[i][transition.second];
			if(cost < bestCost) {
				bestCost = cost.value;
				bestChildState = transition.second;
			}
		}
		Cost selfDefendedCost = Cost(bestCost);
		chosenChildState[i][SELF_DEFENDED] = bestChildState;
		prevParentState[i][SELF_DEFENDED] = SELF_DEFENDED;

		// ====================================================
		// STATE 2: Parent becomes DEFENDING
		// ====================================================
		bestCost = INF;
		bestChildState = INF;
		for(const auto& transition : defendingRomanTransitions) {
			Cost cost = dp[parent][transition.first] + dp[i][transition.second];
			if(cost < bestCost) {
				bestCost = cost.value;
				bestChildState = transition.second;
			}
		}
		Cost defendingCost = Cost(bestCost);
		chosenChildState[i][DEFENDING] = bestChildState;
		prevParentState[i][DEFENDING] = DEFENDING;

		// ====================================================
		// STATE 3: Parent becomes DEFENDED
		// ====================================================
		bestCost = INF;
		bestPreviousParentState = INF;
		bestChildState = INF;
		for(const auto& transition : defendedRomanTransitions) {
			Cost cost = dp[parent][transition.first] + dp[i][transition.second];
			if(cost < bestCost) {
				bestCost = cost.value;
				bestPreviousParentState = transition.first;
				bestChildState = transition.second;
			}
		}
		Cost defendedCost = Cost(bestCost);
		chosenChildState[i][DEFENDED] = bestChildState;
		prevParentState[i][DEFENDED] = bestPreviousParentState;

		// Commit new parent states.
		dp[parent][UNDEFENDED] = undefendedCost;
		dp[parent][SELF_DEFENDED] = selfDefendedCost;
		dp[parent][DEFENDING] = defendingCost;
		dp[parent][DEFENDED] = defendedCost;
	}

	// ====================================================
	// Backtracking Phase: Reconstruct the dominating set
	// ====================================================
	std::vector<int> defendingSet;
	std::vector<int> selfDefendedSet;
	std::vector<int> finalStates(n);

	// The root must end up either SELF_DEFENDED, DEFENDING or DEFENDED
	int finalRootState;

	if(dp[0][SELF_DEFENDED] < dp[0][DEFENDING])
		finalRootState = (dp[0][SELF_DEFENDED] < dp[0][DEFENDED]) ? SELF_DEFENDED : DEFENDED;
	else
		finalRootState = (dp[0][DEFENDING] < dp[0][DEFENDED]) ? DEFENDING : DEFENDED;        
	
	if(finalRootState == DEFENDING) defendingSet.push_back(g.originalIndices[0]);
	if(finalRootState == SELF_DEFENDED) selfDefendedSet.push_back(g.originalIndices[0]);

	finalStates[0] = finalRootState;

	// Traverse top-down to replay the state history
	for (int i = 1; i < n; i++) {
		int parent = g.parentArray[i];
		int currentParentState = finalStates[parent];
		
		// Determine what state this child had to be in
		finalStates[i] = chosenChildState[i][currentParentState];
		if(finalStates[i] == DEFENDING) defendingSet.push_back(g.originalIndices[i]);
		if(finalStates[i] == SELF_DEFENDED) selfDefendedSet.push_back(g.originalIndices[i]);

		// Roll back the parent's state to what it was before this child was added
		finalStates[parent] = prevParentState[i][currentParentState];
	}

	return std::make_pair(selfDefendedSet, defendingSet);
}

bool isRomanDominating(Tree& g, std::vector<int>& selfDefended, std::vector<int> defending ){
	int n = g.neighbourList.size();
	
	std::vector<bool> inDefending(n, false);
	for (int i : defending) {
		inDefending[i] = true;
	}

	std::vector<bool> inSelfDefended(n, false);
	for (int i : selfDefended) {
		inSelfDefended[i] = true;
	}

	for (int i = 0; i < n; i++) {
		if(inDefending[i] || inSelfDefended[i]) continue;
		bool defended = false;
		for(int neighbour : g.neighbourList[i]) {
			if(inDefending[neighbour]) {
				defended = true;
				break;
			}
		}
		if (!defended) return false;
	}
	return true;
}
