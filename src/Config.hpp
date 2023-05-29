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
  std::string getServerProgramName() const;
  std::string getHttpVersion() const;
  std::string getCgiVersion() const;
  std::string getCgiPath() const;
  int getTimeout() const;
  std::map<std::string, std::string> getStausMessages() const;
  std::map<std::string, std::string> getMimeTypes() const;
  std::vector<ServerBlock> getServerBlocks() const;

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
  std::string cgi_path_;
  int timeout_;
  std::map<std::string, std::string> status_messages_;
  std::map<std::string, std::string> mime_types_;
  std::vector<ServerBlock> server_blocks_; 
};

#endif  // SRC_CONFIG_HPP_
