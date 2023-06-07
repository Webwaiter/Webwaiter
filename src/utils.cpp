// Copyright 2023 ean, hanbkim, jiyunpar
#include "src/utils.hpp"

std::vector<std::string> split(std::string input, std::string delimiter_set) {
    std::vector<std::string> result;
    std::string::size_type start = 0;
    std::string::size_type end = input.find_first_of(delimiter_set);
    while (end != std::string::npos) {
        if (end != start) {
            std::string token = input.substr(start, end - start);
            result.push_back(token);
        }
        start = end + 1;
        end = input.find_first_of(delimiter_set, start);
    }
    if (start != input.length()) {
        std::string token = input.substr(start);
        result.push_back(token);
    }
    return result;
}

std::string skipCharset(std::string input, std::string charset) {
	std::string ret;
	size_t pos;
	if ((pos = input.find_first_not_of(charset)) != std::string::npos) {
		ret = input.substr(pos);
		return ret;
	} else {
		return input;
	}
}

void trim(std::string& str)
{
  size_t left_pos = str.find_first_not_of(" ");
  if (left_pos != std::string::npos)
    str.erase(0, left_pos);

  size_t right_pos = str.find_last_not_of(" ");
  if (right_pos != std::string::npos)
    str.erase(right_pos + 1);
}

void updateTime(time_t &cur_time) {
  cur_time = time(NULL);
}

double getTimeOut(time_t &base_time) {
  return (difftime(time(NULL), base_time));
}

in_addr changeIpToBinary(std::string ip) {
  struct in_addr ret;

  std::vector<std::string> ip_token = split(ip, ".");
  ret.s_addr = ((atoi(ip_token[0].c_str()) << 24) | (atoi(ip_token[1].c_str()) << 16)
                | (atoi(ip_token[2].c_str()) << 8) | atoi(ip_token[3].c_str()));
  return ret;
}

std::string changeBinaryToIp(in_addr binary) {
  std::string ip;
  int mask = ~(~0u << 8);
  int nums[4];
  for (int i = 3; i >= 0; --i) {
    nums[i] = binary.s_addr & mask;
    binary.s_addr >>= 8;
  }
  for (int i = 0; i < 3; ++i) {
    ip += numberToString(nums[i]) + '.'; 
  }
  ip.pop_back();
  return ip;
}

void toLower(char &c) {
  c = std::tolower(c);
}

bool deleteFile(const std::string &path) {
  if (unlink(path.c_str()) == 0) {
    return true;
  }
  return false;
}

bool isDirectory(const std::string &path) {
  DIR *dir = opendir(path.c_str());
  if (dir == NULL) {
    return false;
  } else {
    closedir(dir);
    return true;
  }
}
