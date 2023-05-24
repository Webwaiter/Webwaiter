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

class Config {
 public:
  explicit Config(const char *file_path);
  std::string getServerProgramName(void) const;
  std::string getHttpVersion(void) const;
  std::vector<ServerBlock> getServerBlocks(void) const;

 private:
  void parseConfigFile(const char *file_path);
  std::string server_program_name_;
  std::string http_version_;
  std::vector<ServerBlock> server_blocks_; 
};

#endif  // SRC_CONFIG_HPP_