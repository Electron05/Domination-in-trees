#pragma once

#include "tree.h"
#include "bitmask.h"
#include <string>
#include <vector>

std::string generateDotString(const Tree& graph, const std::vector<int>& dominatingSet);
std::string generateDotStringRoman(const Tree& graph, const std::vector<int>& selfDefendedSet, const std::vector<int>& defendingSet);
void writeDotFile(const std::string& variant, const Tree& graph, const std::vector<int>& dominatingSet);
void writeDotFile(const std::string& variant, const Tree& graph, const std::vector<int>& selfDefendedSet, const std::vector<int>& defendingSet);
std::string generateDotStringKRainbow(const Tree& graph, int k, const std::vector<BitMask>& colorMask);
void writeDotFileKRainbow(const Tree& graph, int k, const std::vector<BitMask>& colorMask);
