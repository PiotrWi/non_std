#pragma once

#include <string>
#include <vector>

std::string ltrim(const std::string& in);
std::string rtrim(const std::string& in);
std::string trim(const std::string& in);

std::vector<std::string> splitAndTrim(const std::string& in, char c);
std::vector<std::string> splitAndTrimByStr(const std::string& in, std::string delimeter);

std::vector<std::string> splitNumbersAndLetters(const std::string& in);

