// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_CONFIG_HPP_
#define SRC_CONFIG_HPP_

#include <netinet/in.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

#include "src/ServerBlock.hpp"
#include "src/LocationBlock.hpp"

class Config {
 public:
  explicit Config(const char *file_path);
  std::string getServerProgramName() const;
  std::string getHttpVersion() const;

 private:
  void parseConfigFile(const char *file_path);
  std::string server_program_name_;
  std::string http_version_;
  std::vector<ServerBlock*> server_blocks_; 
};

#endif  // SRC_CONFIG_HPP_