#pragma once
#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

inline std::string last_lines(const std::string &input, int count = 1) {
  std::istringstream f(input);
  std::string line, str;
  std::vector<std::string> lines;

  while (std::getline(f, str)) {
    lines.push_back(str);
  }
  // if (lines.size() <= count) {
  // return input;
  //}
  // std::reverse(lines.begin(), lines.end());
  line += "Lines Count -> " + std::to_string(lines.size() + 1) + "\n";

  for (size_t i = lines.size() - 1 - (size_t)count; i < (size_t)count; i++) {
    line += /*std::to_string(i + 1) + "->" + */ lines[i] + "\n";
  }
  return line;
}