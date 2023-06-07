// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/ServerBlock.hpp"

#include "src/utils.hpp"

ServerBlock::ServerBlock(std::ifstream &file) {
  try {
    parseServerBlock(file);
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
  for (size_t i = 0; i < ip.size(); ++i) {
    for (size_t j = 0; j < ip[i].size(); ++j) {
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

static void checkServerPort(std::string server_port) {
  for (size_t i = 0; i < server_port.size(); ++i) {
    if (!isdigit(server_port[i])) {
      throw FAIL;
    }
  }
  int port = atoi(server_port.c_str());
  if (!(port >= 0 && port <= 65535)) {
    throw FAIL;
  }
}

void ServerBlock::checkSemantics() const {
  checkClientBodySize(client_body_size_);
  checkServerIP(server_ip_);
  checkServerPort(server_port_);
}

void ServerBlock::parseServerBlock(std::ifstream &file) {
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
    } else if (tmp_vec[0] == "client_body_size" && tmp_vec.size() == 2) {
      client_body_size_ = atoi(tmp_vec[1].c_str());
      error_flag |= (1 << 1);
    } else if (tmp_vec[0] == "listen" && tmp_vec.size() == 2) {
      server_ip_ = tmp_vec[1];
      error_flag |= (1 << 2);
    } else if (tmp_vec[0] == "port" && tmp_vec.size() == 2) {
      server_port_ = tmp_vec[1];
      error_flag |= (1 << 3);
    } else if (tmp_vec[0] == "server_name" && tmp_vec.size() == 2) {
      server_name_ = tmp_vec[1];
      error_flag |=  (1 << 4);
    }
  }
  if (!(error_flag == 15 || error_flag == 31) || !brace.empty()) {
    throw FAIL;
  }
}

const int &ServerBlock::getClientBodySize() const {
  return client_body_size_;
}

const std::string &ServerBlock::getServerIP() const {
  return server_ip_;
}

const std::string &ServerBlock::getServerPort() const {
  return server_port_;
}

const std::string &ServerBlock::getServerName() const {
  return server_name_;
}

const std::vector<LocationBlock> &ServerBlock::getLocationBlocks() const {
  return location_blocks_;
}
