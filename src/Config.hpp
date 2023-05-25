// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_CONFIG_HPP_
#define SRC_CONFIG_HPP_

#include <netinet/in.h>

#include <string>
#include <vector>

struct Config {
  explicit Config(const char *file);
  void parseConfig(const char *file);
  
  std::vector<in_port_t> ports_;
  std::string server_name_;
  std::string root_;
};

#endif  // SRC_CONFIG_HPP_
