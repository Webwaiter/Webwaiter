// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/Config.hpp"
#include "src/ReturnState.hpp"
#include "src/utils.hpp"

Config::Config(const char *file_path) {
  try {
    parseConfigFile(file_path);
  }
  catch(int) {
    throw 1;
  }
}

void Config::parseConfigFile(const char *file_path) {
  std::fstream file;
  int error_flag = 0;

  bool isopen = 1;
  file.open(file_path, std::fstream::in);
  isopen = file.is_open();
  if (!file.is_open()) {
    throw 1;
  }
  while (!file.eof()) {
    std::string line;
    std::getline(file, line);
    if (line.find("server {") != std::string::npos) {
      // ServerBlock class construct & push_back to vector
      ServerBlock *server = new ServerBlock(file);
      server_blocks_.push_back(server);
    } else {
      std::vector<std::string> tmp;
      tmp = split(line, " ");
      if (tmp[0] == "program_name") {
        server_program_name_ = tmp[1];
        error_flag |= (1 << 0);
      } else if (tmp[0] == "http_version") {
        http_version_ = tmp[1];
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