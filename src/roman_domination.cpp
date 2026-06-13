#include "tree.h"
#include <vector>

std::vector<int> solveRomanDomination(Tree& g) {
    return std::vector<int>();
}

bool isRomanDominating(Tree& g, std::vector<int>& defending, std::vector<int> selfDefended ){
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
