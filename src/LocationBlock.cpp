// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/LocationBlock.hpp"

#include "src/utils.hpp"

LocationBlock::LocationBlock(std::ifstream &file, std::string url) :url_(url) {
  try {
    parseLocationBlock(file);
    checkSemantics();
  } catch (ReturnState) {
    throw FAIL;
  }
}

static void checkBracePair(std::vector<std::string> &tmp_vec, std::stack<std::string> &brace) {
  for (size_t i = 0; i < tmp_vec.size(); ++i) {
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

static void checkAllowedMethod(const std::set<std::string> &allowed_method) {
  for (std::set<std::string>::const_iterator it = allowed_method.begin(); it != allowed_method.end(); ++it) {
    if (!(*it == "GET" || *it == "POST" || *it == "DELETE")) {
      throw FAIL;
    }
  }
}

static void checkDirectoryListing(const std::string &directory_listting) {
  if (!(directory_listting == "on" || directory_listting == "off")) {
    throw FAIL;
  }
}

static void checkCGIExtention(const std::string &cgi_extention) {
  if (!(cgi_extention == "php" || cgi_extention == "py" || cgi_extention == "bla")) {
    throw FAIL;
  }
}


void LocationBlock::checkSemantics() const {
  checkURL(url_);
  checkRootDir(root_dir_);
  checkAllowedMethod(allowed_method_);
  checkDirectoryListing(directory_listing_);
  checkCGIExtention(cgi_extension_);
}

void LocationBlock::parseLocationBlock(std::ifstream &file) {
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
    } else if (tmp_vec[0] == "redirection" && tmp_vec.size() == 2) {
      redirection_ = tmp_vec[1];
      error_flag |=  (1 << 5);
    }
  }
  if (!(error_flag == 31 || error_flag == 63) || !brace.empty()) {
    throw FAIL;	
  }
}

const std::string &LocationBlock::getUrl() const {
  return url_;
}

const std::string &LocationBlock::getRootDir() const {
  return root_dir_;
}

const std::set<std::string> &LocationBlock::getAllowedMethod() const {
  return allowed_method_;
}

const std::string &LocationBlock::getDirectoryListing() const {
  return directory_listing_;
}

const std::string &LocationBlock::getIndex() const {
  return index_;
}

const std::string &LocationBlock::getCgiExtension() const {
  return cgi_extension_;
}

const std::string &LocationBlock::getRedirection() const {
  return redirection_;
}
