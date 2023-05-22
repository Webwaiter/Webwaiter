// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_LOCATIONBLOCK_HPP_
#define SRC_LOCATIONBLOCK_HPP_

#include <fstream>
#include <string>
#include <vector>
#include <set>

class LocationBlock {
 public:
  explicit LocationBlock(std::fstream &file);

 private: 
  void parseLocationBlock(void);
  std::string url_;
  std::string root_dir_;
  std::set<std::string> allowed_method_;
  std::string redirection_;
  bool directory_listing_;
  std::string index_;
  std::string cgi_extension_;
  std::string cgi_path_;
};

#endif  // SRC_LOCATIONBLOCK_HPP_