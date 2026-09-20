#pragma once

#include "tree.h"
#include "bitmask.h"
#include <vector>

std::vector<BitMask> solveKRainbowDomination(Tree& g, int k);
bool isKRainbowDominating(Tree& g, int k, const std::vector<BitMask>& originalColorMask);
