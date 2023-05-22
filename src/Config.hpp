// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_CONFIG_HPP_
#define SRC_CONFIG_HPP_

#include <netinet/in.h>

#include <string>
#include <vector>
#include <map>

#include "src/ServerBlock.hpp"

class Config {
 public:
  explicit Config(const char *file);

 private:
  std::string server_program_name_;
  std::string http_version_;
  int client_body_size_;
  std::map<std::string, std::string> default_error_pages_;
  std::vector<ServerBlock&> server_blocks_; 
};

#endif  // SRC_CONFIG_HPP_