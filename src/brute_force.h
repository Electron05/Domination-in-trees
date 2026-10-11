#pragma once

#include "tree.h"
#include "variants.h"
#include <string>

int bruteForceMinimumWeight(const VariantInfo& variant, Tree& g, int upperBound);
void runBruteForceTests(const std::string& outputFolder);
