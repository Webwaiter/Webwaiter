
// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_SERVERBLOCK_HPP_
#define SRC_SERVERBLOCK_HPP_

#include <string>
#include <vector>

#include "src/LocationBlock.hpp"

class ServerBlock {
 public:

 private:
  int server_port_;
  std::string server_name_;
  std::vector<LocationBlock&> location_blocks_;
};

#endif  // SRC_SERVERBLOCK_HPP_