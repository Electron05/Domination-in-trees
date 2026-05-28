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

// ====================================================
// Node States definitions
// ====================================================
enum NodeState {
	UNDOMINATED_UNRESTRAINED = 0,
	DOMINATING = 1,
	DOMINATED_UNRESTRAINED = 2,
	UNDOMINATED_RESTRAINED = 3,
	DOMINATED_RESTRAINED = 4,
	STATE_COUNT = 5
};

// Transitions format: {Old Parent State, Child State}
// These dictate how a parent transitions into a new state when a specific child is added.
const std::vector<std::pair<int, int>> undominatedRestrainedTransitions = {
	{UNDOMINATED_UNRESTRAINED, DOMINATED_UNRESTRAINED},
	{UNDOMINATED_UNRESTRAINED, DOMINATED_RESTRAINED},
	{UNDOMINATED_RESTRAINED, DOMINATED_UNRESTRAINED},
	{UNDOMINATED_RESTRAINED, DOMINATED_RESTRAINED}
};

const std::vector<std::pair<int, int>> dominatedRestrainedTransitions = {
	{DOMINATED_UNRESTRAINED, DOMINATED_UNRESTRAINED},
	{DOMINATED_UNRESTRAINED, DOMINATED_RESTRAINED},
	{UNDOMINATED_RESTRAINED, DOMINATING},
	{DOMINATED_RESTRAINED, DOMINATING},
	{DOMINATED_RESTRAINED, DOMINATED_UNRESTRAINED},
	{DOMINATED_RESTRAINED, DOMINATED_RESTRAINED}
};

std::vector<int> solveRestrainedDomination(Tree& g) {
	int n = g.edgeList.size() + 1;

	// Initialize DP table. 
	// dp[node][state] stores the minimum cost of the subtree rooted at 'node' 
	// given that 'node' is in 'state'.
	std::vector<Cost> initialStates(STATE_COUNT);
	initialStates[UNDOMINATED_UNRESTRAINED] = Cost(0); 
	initialStates[DOMINATING]               = Cost(1); // Node is part of dominating set
	initialStates[DOMINATED_UNRESTRAINED]   = Cost::impossible();
	initialStates[UNDOMINATED_RESTRAINED]   = Cost::impossible();
	initialStates[DOMINATED_RESTRAINED]     = Cost::impossible(); 

	std::vector<std::vector<Cost>> dp(n, initialStates);

	// ====================================================
	// Backtracking vectors
	// We must track what the child's state was, and what the parent's state 
	// was before this child was added, to reconstruct the final dominating set.
	// ====================================================
	std::vector<int> stateRow(STATE_COUNT);
	std::vector<std::vector<int>> chosenChildState(n, stateRow);
	std::vector<std::vector<int>> prevParentState(n, stateRow); 

	// child i must be in chosenChildState[i][j] so their parent can be in state j
	// prevParentState[i][DOMINATED_RESTRAINED] = DOMINATED_UNRESTRAINED / UNDOMINATED_RESTRAINED / DOMINATED_RESTRAINED
	// after adding child i to its parent, parent became DOMINATED_RESTRAINED
	// value of this cell determines previous state of the parent
	// its essential to correctly read chosenChildState[i][curentParentState];	

	// Bottom-up traversal
	for(int i = n - 1; i > 0; i--) {
		int parent = g.parentArray[i];
		
		int bestCost;
		int bestChildState;
		int bestPreviousParentState;

		// ====================================================
		// STATE 1: Parent becomes DOMINATING
		// The parent is in the dominating set. 
		// We pick cheapest valid state of the current child.
		// ====================================================
		bestCost = INF;
		bestChildState = INF;
		std::vector<int> possibleChildStates = {DOMINATING, UNDOMINATED_RESTRAINED, DOMINATED_RESTRAINED};
		
		for(int p : possibleChildStates) {
			if(dp[i][p] < bestCost) { 
				bestCost = dp[i][p].value;
				bestChildState = p;
			}
		}
		chosenChildState[i][DOMINATING] = bestChildState;
		prevParentState[i][DOMINATING] = DOMINATING;
		dp[parent][DOMINATING] += dp[i][bestChildState];


		// ====================================================
		// STATE 2: Parent becomes DOMINATED_UNRESTRAINED
		// The parent is dominated by this child. 
		// So this child MUST be DOMINATING. 
		// We pick cheapest valid state of the parent.
		// ====================================================
		bestChildState = DOMINATING;
		bestPreviousParentState = (dp[parent][UNDOMINATED_UNRESTRAINED] < dp[parent][DOMINATED_UNRESTRAINED]) 
							 ? UNDOMINATED_UNRESTRAINED 
							 : DOMINATED_UNRESTRAINED;

		Cost dominatedUnrestrainedCost = std::min(
			dp[parent][UNDOMINATED_UNRESTRAINED] + dp[i][DOMINATING],
			dp[parent][DOMINATED_UNRESTRAINED] + dp[i][DOMINATING]
		);

		chosenChildState[i][DOMINATED_UNRESTRAINED] = bestChildState;
		prevParentState[i][DOMINATED_UNRESTRAINED] = bestPreviousParentState;


		// ====================================================
		// STATE 3: Parent becomes UNDOMINATED_RESTRAINED
		// Evaluate all valid transition pairs from the lookup table.
		// ====================================================
		bestCost = INF;
		for(const auto& transition : undominatedRestrainedTransitions) {
			Cost cost = dp[parent][transition.first] + dp[i][transition.second];
			if(cost < bestCost) {
				bestCost = cost.value;
				bestPreviousParentState = transition.first;
				bestChildState = transition.second;
			}
		}
		Cost undominatedRestrainedCost = Cost(bestCost);
		chosenChildState[i][UNDOMINATED_RESTRAINED] = bestChildState;
		prevParentState[i][UNDOMINATED_RESTRAINED] = bestPreviousParentState;


		// ====================================================
		// STATE 4: Parent becomes DOMINATED_RESTRAINED
		// Evaluate all valid transition pairs from the lookup table.
		// ====================================================
		bestCost = INF;
		for(const auto& transition : dominatedRestrainedTransitions) {
			Cost cost = dp[parent][transition.first] + dp[i][transition.second];
			if(cost < bestCost) {
				bestCost = cost.value;
				bestPreviousParentState = transition.first;
				bestChildState = transition.second;
			}
		}
		Cost dominatedRestrainedCost = Cost(bestCost);
		chosenChildState[i][DOMINATED_RESTRAINED] = bestChildState;
		prevParentState[i][DOMINATED_RESTRAINED] = bestPreviousParentState;

		// Commit new parent states.
		dp[parent][UNDOMINATED_UNRESTRAINED] = Cost::impossible();
		dp[parent][DOMINATED_UNRESTRAINED] = dominatedUnrestrainedCost;
		dp[parent][UNDOMINATED_RESTRAINED] = undominatedRestrainedCost;
		dp[parent][DOMINATED_RESTRAINED] = dominatedRestrainedCost;
	
	}
	
	// ====================================================
	// Backtracking Phase: Reconstruct the dominating set
	// ====================================================
	std::vector<int> dominatingSet;
	std::vector<int> finalStates(n);

	// The root must end up either DOMINATING or properly DOMINATED_RESTRAINED
	int finalRootState = (dp[0][DOMINATING] < dp[0][DOMINATED_RESTRAINED]) ? DOMINATING : DOMINATED_RESTRAINED;	 
	finalStates[0] = finalRootState;
	
	if(finalRootState == DOMINATING) dominatingSet.push_back(g.originalIndices[0]);

	// Traverse top-down to replay the state history
	for (int i = 1; i < n; i++) {
		int parent = g.parentArray[i];
		int currentParentState = finalStates[parent];
		
		// Determine what state this child had to be in
		finalStates[i] = chosenChildState[i][currentParentState];
		if(finalStates[i] == DOMINATING) {
			dominatingSet.push_back(g.originalIndices[i]);
		}

		// Roll back the parent's state to what it was before this child was added
		finalStates[parent] = prevParentState[i][currentParentState];
	}

	return dominatingSet;
}