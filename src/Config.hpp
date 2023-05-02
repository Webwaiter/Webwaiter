// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_CONFIG_HPP_
#define SRC_CONFIG_HPP_

#include <netinet/in.h>

#include <vector>

struct Config {
  explicit Config(const char *file);
  void parseConfig(const char *file);
  
  std::vector<in_port_t> ports_;
};

#endif  // SRC_CONFIG_HPP_