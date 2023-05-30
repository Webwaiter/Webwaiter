// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_UTILS_HPP_
#define SRC_UTILS_HPP_

#include <sys/time.h>

#include <string>
#include <vector>

enum ReturnState {
  SUCCESS,
  FAIL,
  AGAIN,
  CONNECTION_CLOSE
};

std::vector<std::string> split(std::string input, std::string delimiter);
std::string skipCharset(std::string input, std::string charset);
void trim(std::string& str);
long getTimeOut(struct timeval base_timeval);


#endif //SRC_UTILS_HPP_