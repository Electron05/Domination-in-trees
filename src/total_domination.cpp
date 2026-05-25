#include "tree.h"
#include <vector>

// https://cocalc.com/share/download/e948e806e1d2350983bbf3b4bbabbb5a1b430fd8/Papers/henning-survey-total-domination-2009.pdf

enum class VertexLabel : char {
	Bound,
	Free,
	Required,
	Needed
};


std::vector<int> solveTotalDomination(Tree& g) {
	int n = g.edgeList.size() + 1;

	std::vector<VertexLabel> labels(n, VertexLabel::Bound);

	std::vector<int> totallyDominatingSet;

	if (g.edgeList.empty())
		return;

	for(int i = n-1; i > 0; i--) {
		if (labels[i] == VertexLabel::Bound &&
			labels[g.parentArray[i]] == VertexLabel::Bound)
			labels[g.parentArray[i]] = VertexLabel::Required;

		else if (labels[i] == VertexLabel::Bound &&
				 labels[g.parentArray[i]] == VertexLabel::Free)
			labels[g.parentArray[i]] = VertexLabel::Needed;

		else if (labels[i] == VertexLabel::Required) {
			totallyDominatingSet.push_back(i);
			labels[g.parentArray[i]] = VertexLabel::Needed;
		}

		else if (labels[i] == VertexLabel::Needed && 
				 labels[g.parentArray[i]] == VertexLabel::Bound) {
			totallyDominatingSet.push_back(i);
			labels[g.parentArray[i]] = VertexLabel::Free;
		}

		else if (labels[i] == VertexLabel::Needed && 
				 labels[g.parentArray[i]] == VertexLabel::Required) {
			totallyDominatingSet.push_back(i);
			labels[g.parentArray[i]] = VertexLabel::Needed;
		}
	}

	if(labels[0] == VertexLabel::Needed)
		totallyDominatingSet.push_back(0);
	else if(labels[0] == VertexLabel::Required) {
		totallyDominatingSet.push_back(0);
		totallyDominatingSet.push_back(1);
	}
	else if(labels[0] == VertexLabel::Bound)
		totallyDominatingSet.push_back(1);

	return totallyDominatingSet;
} 