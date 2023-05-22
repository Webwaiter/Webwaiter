
// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_LOCATIONBLOCK_HPP_
#define SRC_LOCATIONBLOCK_HPP_

#include <string>
#include <vector>
#include <set>

class LocationBlock {
 public:

 private:
  std::string root_dir_;
  std::set<std::string> allowed_method_;
  std::string redirection_;
  bool directory_listing_;
  std::string index_;
  std::string cgi_extension_;  
};

#endif  // SRC_LOCATIONBLOCK_HPP_