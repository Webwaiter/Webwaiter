// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_LOCATIONBLOCK_HPP_
#define SRC_LOCATIONBLOCK_HPP_

#include <dirent.h>
#include <unistd.h>

#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include <set>

class LocationBlock {
 public:
  explicit LocationBlock(std::fstream &file, std::string url);
  const std::string &getUrl() const;
  const std::string &getRootDir() const;
  const std::set<std::string> &getAllowedMethod() const;
  const std::string &getDirectoryListing() const;
  const std::string &getIndex() const;
  const std::string &getCgiExtension() const;
  const std::string &getRedirection() const;

 private: 
  void parseLocationBlock(std::fstream &file);
  void checkSemantics() const;
  std::string url_;
  std::string root_dir_;
  std::set<std::string> allowed_method_;
  std::string directory_listing_;
  std::string index_;
  std::string cgi_extension_;
  std::string redirection_;
};

#endif  // SRC_LOCATIONBLOCK_HPP_