// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_CONFIG_HPP_
#define SRC_CONFIG_HPP_

#include <netinet/in.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <stack>

#include "src/ServerBlock.hpp"

class Config {
 public:
  explicit Config(const char *file_path);
  std::string getServerProgramName(void) const;
  std::string getHttpVersion(void) const;
  std::vector<ServerBlock> getServerBlocks(void) const;

 private:
  void parseConfigFile(const char *file_path);
  void parseStatusFile(const char *file_path);
  void parseMIMEFile(const char *file_path);
  void checkSemantics(void) const;
  std::string server_program_name_;
  std::string http_version_;
  std::string status_path_;
  std::string mime_path_;
  std::map<int, std::string> status_messages_;
  std::map<std::string, std::string> mime_types_;
  std::vector<ServerBlock> server_blocks_; 
};

#endif  // SRC_CONFIG_HPP_
