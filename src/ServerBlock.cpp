// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/ServerBlock.hpp"
#include "src/ReturnState.hpp"
#include "src/utils.hpp"

ServerBlock::ServerBlock(std::fstream &file) {
  try {
    parseServerBlock(file);
  } catch (int) {
    throw FAIL;
  }
}

void ServerBlock::parseServerBlock(std::fstream &file) {
  int error_flag = 0;
  std::stack<std::string> brace;
  brace.push("{");
  while (!brace.empty()) {
    std::string line;
    std::getline(file, line);
    std::string tmp = skipCharset(line, " \t");
    std::vector<std::string> tmp_vec = split(tmp, " ");
    // eof when not paired brace -> error
    if (tmp == "}") {
      if (brace.top() == "{") {
        brace.pop();
      }
    } else if (tmp.find("location") != std::string::npos) {
      // url parsing 불가시 error -> tmp_vec size로 파
      LocationBlock *location = new LocationBlock(file, tmp_vec[1]);
      location_blocks_.push_back(location);
      error_flag |= (1 << 5);
    } else {
      if (tmp_vec[0] == "default_error_page_400") {
        default_error_pages_["400"] = tmp_vec[1];
        error_flag |= (1 << 0);
      } else if (tmp_vec[0] == "default_error_page_500") {
        default_error_pages_["500"] = tmp_vec[1];
        error_flag |= (1 << 1);
      } else if (tmp_vec[0] == "client_body_size") {
        client_body_size_ = atoi(tmp_vec[1].c_str());
        error_flag |= (1 << 2);
      } else if (tmp_vec[0] == "listen") {
        server_ip_ = tmp_vec[1];
        error_flag |= (1 << 3);
      } else if (tmp_vec[0] == "port") {
        server_port_ = atoi(tmp_vec[1].c_str());
        error_flag |= (1 << 4);
      } else if (tmp_vec[0] == "server_name") {
        server_name_ = tmp_vec[1];
        error_flag |=  (1 << 6);
      }
    }
  }
  if (error_flag != 63 && error_flag != 127) {
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