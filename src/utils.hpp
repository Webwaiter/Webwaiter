// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_UTILS_HPP_
#define SRC_UTILS_HPP_

#include <sstream>
#include <string>
#include <vector>
#include <ctime>

static const char kCrlf[] = {'\r', '\n'};
static const size_t kCrlfLength = 2;

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

template <typename T>
std::string numberToString(T number) {
  std::stringstream ss;
  ss << number;
  return ss.str();
}

template<typename T>
void appendCrlf(T &dest) {
    dest.insert(dest.end(), kCrlf, kCrlf + kCrlfLength);
}

template<typename T , typename U>
void appendData(T &dest, U &src) {
  dest.insert(dest.end(), src.begin(), src.end());
}

#endif //SRC_UTILS_HPP_