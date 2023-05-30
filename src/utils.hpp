// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_UTILS_HPP_
#define SRC_UTILS_HPP_

#include <string>
#include <vector>
#include <ctime>

enum ReturnState {
  SUCCESS,
  FAIL,
  AGAIN,
  CONNECTION_CLOSE
};

std::vector<std::string> split(std::string input, std::string delimiter);
std::string skipCharset(std::string input, std::string charset);
void trim(std::string& str);
void updateTime(time_t &cur_time);
double getTimeOut(time_t &base_time);

#endif //SRC_UTILS_HPP_