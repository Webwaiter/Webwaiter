// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_UTILS_HPP_
#define SRC_UTILS_HPP_

#include <netinet/in.h>
#include <unistd.h>
#include <dirent.h>

#include <sstream>
#include <string>
#include <vector>
#include <ctime>

static const char kCrlf[] = {'\r', '\n'};
static const char kNl[] = {'\n'};
static const size_t kCrlfLength = 2;
static const size_t kNlLength = 1;

enum ReturnState {
  SUCCESS,
  FAIL,
  AGAIN,
  CONNECTION_CLOSE,
  TIMEOUT,
  SYSTEM_OVERLOAD
};

std::vector<std::string> split(std::string input, std::string delimiter);
std::string skipCharset(std::string input, std::string charset);
void trim(std::string& str);

void updateTime(time_t &cur_time);
double getTimeOut(time_t &base_time);

in_addr changeIpToBinary(std::string ip);
std::string changeBinaryToIp(in_addr binary);
void toLower(char &c);
bool deleteFile(const std::string &path);
bool isDirectory(const std::string &path);
bool isResponseOk(int status_code);

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
