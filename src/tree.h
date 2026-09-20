#pragma once

#include <vector>
#include <tuple>
#include <iosfwd>
#include <string>
#include "bitmask.h"

#define PARENT_ARRAY_INIT_VALUE -2
#define PARENT_ARRAY_ROOT_PARENT -1
#define OLD_TO_NEW_IDEX_INIT -1

struct Tree {
    std::vector<std::tuple<int, int>> edgeList;
    std::vector<int> parentArray;
    std::vector<int> parentArrayIndices; // original vertex index -> new parentArray index
    std::vector<int> originalIndices;    // parentArray index -> original vertex index
    std::vector<std::vector<int>> neighbourList;
};

void openNthTree(std::ifstream& plik, int n, Tree& graph);
void edgesToParentArray(Tree& graph);
void printEdgeList(const Tree& graph);
void printParentArray(const Tree& graph);
void printOriginalToParentArrayIndices(const Tree& graph);
void printDominatingSet(int variant, std::vector<int>& set);
