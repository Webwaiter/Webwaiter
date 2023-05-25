// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/LocationBlock.hpp"

#include "src/utils.hpp"

LocationBlock::LocationBlock(std::fstream &file, std::string url) :url_(url) {
  try {
    parseLocationBlock(file);
    checkSemantics();
  } catch (int) {
    throw FAIL;
  }
}

static void checkBracePair(std::vector<std::string> &tmp_vec, std::stack<std::string> &brace) {
  for (int i = 0; i < tmp_vec.size(); ++i) {
    if (tmp_vec[i] == "{") {
      brace.push("{");
    } else if (tmp_vec[i] == "}") {
      if (brace.empty()) {
        throw FAIL;
      } else if (brace.top() == "{") {
        brace.pop();
      }
    }
  }
}

static void checkURL(const std::string &url) {
  if (url[0] != '/') {
    throw FAIL;
  }
}

static void checkRootDir(const std::string &root_dir) {
  DIR *dir = opendir(root_dir.c_str());
  if (dir == NULL) {
    throw FAIL;
  } else {
    closedir(dir);
  }
}

static void checkAllowMethod(const std::set<std::string> &allowed_method) {
  for (std::set<std::string>::const_iterator it; it != allowed_method.end(); ++it) {
    if (*it != "GET" || *it != "POST" || *it != "DELETE") {
      throw FAIL;
    }
  }
}

static void checkDirectoryListing(const std::string &directory_listting) {
  if (directory_listting != "on" || directory_listting != "off") {
    throw FAIL;
  }
}

static void checkCGIExtention(const std::string &cgi_extention) {
  if (cgi_extention != "php" | cgi_extention != "py") {
    throw FAIL;
  }
}

static void checkCGIPath(const std::string &cgi_path) {
  if (access(cgi_path.c_str(), X_OK | F_OK) == -1) {
    throw FAIL;
  }
}

void LocationBlock::checkSemantics(void) const {
  checkURL(url_);
  checkRootDir(root_dir_);
  checkAllowMethod(allowed_method_);
  checkDirectoryListing(directory_listing_);
  checkCGIExtention(cgi_extension_);
  checkCGIPath(cgi_path_);
}

void LocationBlock::parseLocationBlock(std::fstream &file) {
  int error_flag = 0;
  std::stack<std::string> brace;
  brace.push("{");
  while (!file.eof() && !brace.empty()) {
    std::string line;
    std::getline(file, line);
    std::string tmp = skipCharset(line, " \t");
    std::vector<std::string> tmp_vec = split(tmp, " \t");
    checkBracePair(tmp_vec, brace);
    if (tmp == "") {
      continue;
    } else if (tmp_vec[0] == "root_dir" && tmp_vec.size() == 2) {
      root_dir_ = tmp_vec[1];
      error_flag |= (1 << 0);
    } else if (tmp_vec[0] == "allowed_method") {
      for (size_t i = 1; i < tmp_vec.size(); ++i) {
        allowed_method_.insert(tmp_vec[i]);
      }
      error_flag |= (1 << 1);
    } else if (tmp_vec[0] == "directory_listing" && tmp_vec.size() == 2) {
      directory_listing_ = tmp_vec[1];
      error_flag |= (1 << 2);
    } else if (tmp_vec[0] == "index" && tmp_vec.size() == 2) {
      index_ = tmp_vec[1];
      error_flag |= (1 << 3);
    } else if (tmp_vec[0] == "cgi_extension" && tmp_vec.size() == 2) {
      cgi_extension_ = tmp_vec[1];
      error_flag |= (1 << 4);
    } else if (tmp_vec[0] == "cgi_path" && tmp_vec.size() == 2) {
      cgi_path_ = tmp_vec[1];
      error_flag |=  (1 << 5);
    } else if (tmp_vec[0] == "redirection" && tmp_vec.size() == 2) {
      redirection_ = tmp_vec[1];
      error_flag |=  (1 << 6);
    }
  }
  if (error_flag != 63 && error_flag != 127 && !brace.empty()) {
    throw FAIL;	
  }
}
