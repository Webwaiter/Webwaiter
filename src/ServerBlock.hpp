// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_SERVERBLOCK_HPP_
#define SRC_SERVERBLOCK_HPP_

#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <stack>
#include <cstdlib>

#include "src/LocationBlock.hpp"

class ServerBlock {
 public:
  explicit ServerBlock(std::fstream &file);
  std::map<std::string, std::string> getDefaultErrorPages(void) const;
  int getClientBodySize(void) const;
  std::string getServerIP(void) const;
  int getServerPort(void) const;
  std::string getServerName(void) const;

 private:
  void parseServerBlock(std::fstream &file);
  void checkSemantics(void) const;
  std::map<std::string, std::string> default_error_pages_;
  int client_body_size_;
  std::string server_ip_;
  int server_port_;
  std::string server_name_;
  std::vector<LocationBlock> location_blocks_;
};

#endif  // SRC_SERVERBLOCK_HPP_