// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/ServerBlock.hpp"

#include "src/utils.hpp"

ServerBlock::ServerBlock(std::fstream &file) {
  try {
    parseServerBlock(file);
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

void ServerBlock::checkSemantics(void) const {

}
void ServerBlock::parseServerBlock(std::fstream &file) {
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
    } else if (tmp_vec[0] == "location" && tmp_vec.size() == 3) {
      if (tmp_vec[2] != "{") {
        throw FAIL;
      }
      location_blocks_.push_back(LocationBlock(file, tmp_vec[1]));
      brace.pop();
      error_flag |= (1 << 0);
    } else if (tmp_vec[0] == "default_error_page_400" && tmp_vec.size() == 2) {
      default_error_pages_["400"] = tmp_vec[1];
      error_flag |= (1 << 1);
    } else if (tmp_vec[0] == "default_error_page_500" && tmp_vec.size() == 2) {
      default_error_pages_["500"] = tmp_vec[1];
      error_flag |= (1 << 2);
    } else if (tmp_vec[0] == "client_body_size" && tmp_vec.size() == 2) {
      client_body_size_ = atoi(tmp_vec[1].c_str());
      error_flag |= (1 << 3);
    } else if (tmp_vec[0] == "listen" && tmp_vec.size() == 2) {
      server_ip_ = tmp_vec[1];
      error_flag |= (1 << 4);
    } else if (tmp_vec[0] == "port" && tmp_vec.size() == 2) {
      server_port_ = atoi(tmp_vec[1].c_str());
      error_flag |= (1 << 5);
    } else if (tmp_vec[0] == "server_name" && tmp_vec.size() == 2) {
      server_name_ = tmp_vec[1];
      error_flag |=  (1 << 6);
    }
  }
  if (error_flag != 63 && error_flag != 127 && !brace.empty()) {
    throw FAIL;
  }
}

std::map<std::string, std::string> ServerBlock::getDefaultErrorPages(void) const {
  return default_error_pages_;
}

int ServerBlock::getClientBodySize(void) const {
  return client_body_size_;
}

std::string ServerBlock::getServerIP(void) const {
  return server_ip_;
}

int ServerBlock::getServerPort(void) const {
  return server_port_;
}

std::string ServerBlock::getServerName(void) const {
  return server_name_;
}