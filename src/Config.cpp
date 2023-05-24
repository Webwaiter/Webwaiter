// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/Config.hpp"
#include "src/ReturnState.hpp"
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

  file.open(file_path, std::fstream::in);
  if (!file.is_open()) {
    throw FAIL;
  }
  while (!file.eof()) {
    std::string line;
    std::getline(file, line);
    std::vector<std::string> tmp_vec = split(line, " \t");
    if (line == "") {
      continue;
    }
    if (line.find("server {") != std::string::npos) {
      // ServerBlock class construct & push_back to vector
      server_blocks_.push_back(ServerBlock(file));
    } else {
      if (tmp_vec[0] == "program_name") {
        server_program_name_ = tmp_vec[1];
        error_flag |= (1 << 0);
      } else if (tmp_vec[0] == "http_version") {
        http_version_ = tmp_vec[1];
        error_flag |= (1 << 1);
      }
    }
  }
  if (error_flag != 3) {
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