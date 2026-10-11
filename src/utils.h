#pragma once

#include <vector>
#include <string>
#include <iosfwd>

std::vector<int> parseIntegerList(const char* envVal);
int parseInt(const char* envVal);
void writeLog(std::ofstream& logFile, const std::string& msg, bool newline = true);
std::string padLeft(const std::string& s, size_t width);
std::string padRight(const std::string& s, size_t width);
