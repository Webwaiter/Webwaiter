// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_UTILS_HPP_
#define SRC_UTILS_HPP_

#include <string>
#include <vector>

std::vector<std::string> split(std::string input, std::string delimiter);
std::string skipCharset(std::string input, std::string charset);

#endif //SRC_UTILS_HPP_