#ifndef TREE_H
#define TREE_H

#include <vector>
#include <tuple>

#define PARENT_ARRAY_INIT_VALUE -2
#define PARENT_ARRAY_ROOT_PARENT -1
#define OLD_TO_NEW_IDEX_INIT -1

struct Tree {
    std::vector<std::tuple<int, int>> edgeList;
    std::vector<int> parentArray;
    std::vector<int> parentArrayIndices; // original vertex index -> new parentArray index
    std::vector<int> originalIndices; // parentArray index -> orginal vertex index
};

#endif
