// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/LocationBlock.hpp"
#include "src/ReturnState.hpp"
#include "src/utils.hpp"

LocationBlock::LocationBlock(std::fstream &file, std::string url) :url_(url) {
  try {
    parseLocationBlock(file);
  } catch (int) {
    throw FAIL;
  }
}

void LocationBlock::parseLocationBlock(std::fstream &file) {
  int error_flag = 0;
  std::stack<std::string> brace;
  brace.push("{");
  while (!brace.empty()) {
    std::string line;
    std::getline(file, line);
    std::string tmp = skipCharset(line, " \t");
    std::vector<std::string> tmp_vec = split(tmp, " \t");
    // TODO:check error_case 
    if (tmp == "") {
      continue;
    }
    if (tmp == "}") {
      if (brace.top() == "{") {
        brace.pop();
      }
    } else {
      if (tmp_vec[0] == "root_dir") {
        root_dir_ = tmp_vec[1];
        error_flag |= (1 << 0);
      } else if (tmp_vec[0] == "allowed_method") {
        for (size_t i = 1; i < tmp_vec.size(); ++i) {
          allowed_method_.insert(tmp_vec[i]);
        }
        error_flag |= (1 << 1);
      } else if (tmp_vec[0] == "directory_listing") {
        if (tmp_vec[1] == "on") {
          directory_listing_ = true;
          error_flag |= (1 << 2);
        } else if (tmp_vec[1] == "off") {
          directory_listing_ = false;
          error_flag |= (1 << 2);
        } else {
          throw FAIL;
        }
      } else if (tmp_vec[0] == "index") {
        index_ = tmp_vec[1];
        error_flag |= (1 << 3);
      } else if (tmp_vec[0] == "cgi_extension") {
        cgi_extension_ = tmp_vec[1];
        error_flag |= (1 << 4);
      } else if (tmp_vec[0] == "cgi_path") {
        cgi_path_ = tmp_vec[1];
        error_flag |=  (1 << 5);
      } else if (tmp_vec[0] == "redirection") {
        redirection_ = tmp_vec[1];
        error_flag |=  (1 << 6);
      }
    }
  }
  if (error_flag != 63 && error_flag != 127) {
    throw FAIL;	
  }
}