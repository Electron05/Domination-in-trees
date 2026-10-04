#pragma once

#include "tree.h"
#include "bitmask.h"
#include <string>
#include <vector>

std::string generateDotString(const Tree& graph, const std::vector<int>& dominatingSet);
std::string generateDotStringRoman(const Tree& graph, const std::vector<int>& selfDefendedSet, const std::vector<int>& defendingSet);
std::string generateDotStringKRainbow(const Tree& graph, int k, const std::vector<BitMask>& colorMask);

void writeDotFile(const std::string& filePath, const Tree& graph, const std::vector<int>& dominatingSet);
void writeDotFileRoman(const std::string& filePath, const Tree& graph, const std::vector<int>& selfDefendedSet, const std::vector<int>& defendingSet);
void writeDotFileKRainbow(const std::string& filePath, const Tree& graph, int k, const std::vector<BitMask>& colorMask);

void generatePreviews(const std::string& outputFolder);
