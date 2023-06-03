// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_SERVERBLOCK_HPP_
#define SRC_SERVERBLOCK_HPP_

#include <unistd.h>

#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <stack>
#include <cstdlib>
#include <cctype>

#include "src/LocationBlock.hpp"

class ServerBlock {
 public:
  explicit ServerBlock(std::ifstream &file);
  const std::map<std::string, std::string> &getDefaultErrorPages(void) const;
  const int &getClientBodySize() const;
  const std::string &getServerIP() const;
  const std::string &getServerPort() const;
  const std::string &getServerName() const;
  const std::vector<LocationBlock> &getLocationBlocks() const;

 private:
  void parseServerBlock(std::ifstream &file);
  void checkSemantics() const;
  std::map<std::string, std::string> default_error_pages_;
  int client_body_size_;
  std::string server_ip_;
  std::string server_port_;
  std::string server_name_;
  std::vector<LocationBlock> location_blocks_;
};

#endif  // SRC_SERVERBLOCK_HPP_
