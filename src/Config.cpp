// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/Config.hpp"
#include "src/utils.hpp"

Config::Config(const char *file_path) {
  try {
    parseConfigFile(file_path);
  }
  catch(int) {
    throw FAIL;
  }
}

void Config::parseConfigFile(const char *file_path) {
  std::fstream file;
  int error_flag = 0;
  std::stack<std::string> brace;

  file.open(file_path, std::fstream::in);
  if (!file.is_open()) {
    throw FAIL;
  }
  while (!file.eof()) {
    std::string line;
    std::getline(file, line);
    std::string tmp = skipCharset(line, " \t");
    std::vector<std::string> tmp_vec = split(line, " \t");
    for (int i = 0; i < tmp_vec.size(); ++i) {
      if (tmp_vec[i] == "{") {
        brace.push("{");
      } else if (tmp_vec[i] == "}") {
        if (brace.empty()) {
          throw FAIL;
        } else if (brace.top() == "{") {
          brace.pop();
        }
      }
    }
    if (tmp == "") {
      continue;
    }
    if (tmp_vec[0] == "server" && tmp_vec.size() == 2) {
      if (tmp_vec[1] != "{") {
        throw FAIL;
      }
      server_blocks_.push_back(ServerBlock(file));
      error_flag |= (1 << 0);
    } else {
      if (tmp_vec[0] == "program_name" && tmp_vec.size() == 2) {
        server_program_name_ = tmp_vec[1];
        error_flag |= (1 << 1);
      } else if (tmp_vec[0] == "http_version" && tmp_vec.size() == 2) {
        http_version_ = tmp_vec[1];
        error_flag |= (1 << 2);
      }
    }
  }
  if (error_flag != 7 && !brace.empty()) {
    throw FAIL;
  }
}

std::string Config::getServerProgramName(void) const {
  return server_program_name_;
}

std::string Config::getHttpVersion(void) const {
  return http_version_;
}

std::vector<ServerBlock> Config::getServerBlocks(void) const {
  return server_blocks_;
}