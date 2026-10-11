#include "utils.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

std::vector<int> parseIntegerList(const char* envVal) {
	std::vector<int> result;
	if (!envVal) return result;
	std::stringstream ss(envVal);
	int val;
	while (ss >> val) {
		result.push_back(val);
	}
	return result;
}

int parseInt(const char* envVal) {
	if (!envVal) return 0;
	return std::stoi(envVal);
}

void writeLog(std::ofstream& logFile, const std::string& msg, bool newline) {
	std::cout << msg << (newline ? "\n" : "") << std::flush;
	if (logFile.is_open()) {
		logFile << msg << (newline ? "\n" : "");
		logFile.flush();
	}
}

std::string padLeft(const std::string& s, size_t width) {
	return std::string(width > s.length() ? width - s.length() : 0, ' ') + s;
}

std::string padRight(const std::string& s, size_t width) {
	return s + std::string(width > s.length() ? width - s.length() : 0, ' ');
}
