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
static void checkDefaultErrorPage(std::map<std::string, std::string> default_error_page) {
  if (access(default_error_page["200"].c_str(), R_OK | F_OK) == -1) {
    throw FAIL; 
  }
  if (access(default_error_page["300"].c_str(), R_OK | F_OK) == -1) {
    throw FAIL; 
  }
  if (access(default_error_page["400"].c_str(), R_OK | F_OK) == -1) {
    throw FAIL; 
  }
  if (access(default_error_page["500"].c_str(), R_OK | F_OK) == -1) {
    throw FAIL; 
  }
}

static void checkClientBodySize(int client_body_size) {
  if (client_body_size <= 0 || client_body_size > 10485760) {
    throw FAIL;
  }
}

static void checkServerIP(std::string server_ip) {
  std::vector<std::string> ip;

  ip = split(server_ip, ".");
  if (ip.size() != 4) {
    throw FAIL;
  }
  for (int i = 0; i < ip.size(); ++i) {
    for (int j = 0; j < ip[i].size(); ++j) {
      if (!isdigit(ip[i][j])) {
        throw FAIL;
      }
    }
    long token = strtol(ip[i].c_str(), NULL, 10);
    if (!(token >= 0 && token <= 255)) {
      throw FAIL;
    }
  }
}

static void checkServerPort(int server_port) {
  if (!(server_port >= 0 && server_port <= 65535)) {
    throw FAIL;
  }
}

void ServerBlock::checkSemantics() const {
  checkDefaultErrorPage(default_error_pages_);
  checkClientBodySize(client_body_size_);
  checkServerIP(server_ip_);
  checkServerPort(server_port_);
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
    } else if (tmp_vec[0] == "default_error_page_200" && tmp_vec.size() == 2) {
      default_error_pages_["200"] = tmp_vec[1];
      error_flag |= (1 << 1);
    } else if (tmp_vec[0] == "default_error_page_300" && tmp_vec.size() == 2) {
      default_error_pages_["300"] = tmp_vec[1];
      error_flag |= (1 << 2);
    } else if (tmp_vec[0] == "default_error_page_400" && tmp_vec.size() == 2) {
      default_error_pages_["400"] = tmp_vec[1];
      error_flag |= (1 << 3);
    } else if (tmp_vec[0] == "default_error_page_500" && tmp_vec.size() == 2) {
      default_error_pages_["500"] = tmp_vec[1];
      error_flag |= (1 << 4);
    } else if (tmp_vec[0] == "client_body_size" && tmp_vec.size() == 2) {
      client_body_size_ = atoi(tmp_vec[1].c_str());
      error_flag |= (1 << 5);
    } else if (tmp_vec[0] == "listen" && tmp_vec.size() == 2) {
      server_ip_ = tmp_vec[1];
      error_flag |= (1 << 6);
    } else if (tmp_vec[0] == "port" && tmp_vec.size() == 2) {
      server_port_ = atoi(tmp_vec[1].c_str());
      error_flag |= (1 << 7);
    } else if (tmp_vec[0] == "server_name" && tmp_vec.size() == 2) {
      server_name_ = tmp_vec[1];
      error_flag |=  (1 << 8);
    }
  }
  if (!(error_flag == 255 || error_flag == 511) || !brace.empty()) {
    throw FAIL;
  }
}

const std::map<std::string, std::string> &ServerBlock::getDefaultErrorPages(void) const {
  return default_error_pages_;
}

const int &ServerBlock::getClientBodySize() const {
  return client_body_size_;
}

const std::string &ServerBlock::getServerIP() const {
  return server_ip_;
}

const int &ServerBlock::getServerPort() const {
  return server_port_;
}

const std::string &ServerBlock::getServerName() const {
  return server_name_;
}

const std::vector<LocationBlock> &ServerBlock::getLocationBlocks() const {
  return location_blocks_;
}
