#pragma once

#include "tree.h"
#include "bitmask.h"
#include <vector>

std::vector<BitMask> solveKRainbowDomination(Tree& g, int k);
bool isVertexKRainbowDominated(int v, int k, const std::vector<int>& neighbours, const std::vector<BitMask>& colorMask);
bool isKRainbowDominating(Tree& g, int k, const std::vector<BitMask>& originalColorMask);
