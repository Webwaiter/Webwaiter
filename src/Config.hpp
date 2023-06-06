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
  const std::string &getServerProgramName() const;
  const std::string &getHttpVersion() const;
  const std::string &getCgiVersion() const;
  const int &getTimeout() const;
  const std::map<std::string, std::string> &getStausMessages() const;
  const std::map<std::string, std::string> &getMimeTypes() const;
  const std::vector<ServerBlock> &getServerBlocks() const;

 private:
  void parseConfigFile(const char *file_path);
  void parseStatusFile(const char *file_path);
  void parseMimeFile(const char *file_path);
  void checkSemantics() const;
  std::string server_program_name_;
  std::string http_version_;
  std::string cgi_version_;
  std::string status_path_;
  std::string mime_path_;
  int timeout_;
  std::map<std::string, std::string> status_messages_;
  std::map<std::string, std::string> mime_types_;
  std::vector<ServerBlock> server_blocks_; 
};

#endif  // SRC_CONFIG_HPP_
