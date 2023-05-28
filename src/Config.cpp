// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/Config.hpp"

#include "src/utils.hpp"

Config::Config(const char *file_path) {
  try {
    parseConfigFile(file_path);
    checkSemantics();
  }
  catch(int) {
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

static void checkHTTPVersion(std::string http_version) {
  if (http_version != "1.1") {
    throw FAIL;
  }
}

static void checkStatusPath(std::string status_path) {
  if (access(status_path.c_str(), R_OK | F_OK) == -1) {
    throw FAIL;
  }
}

static void checkMIMEPath(std::string mime_path) {
  if (access(mime_path.c_str(), R_OK | F_OK) == -1) {
    throw FAIL;
  }
}

void Config::checkSemantics(void) const {
  checkHTTPVersion(http_version_);
  checkStatusPath(status_path_);
  checkMIMEPath(mime_path_);
}

void Config::parseConfigFile(const char *file_path) {
  std::fstream file;
  int error_flag = 0;
  std::stack<std::string> brace;

  file.open(file_path, std::fstream::in);
  if (!file.is_open()) {
    throw FAIL;
  }
  while (!file.eof()) {
    std::string line;
    std::getline(file, line);
    std::string tmp = skipCharset(line, " \t");
    std::vector<std::string> tmp_vec = split(line, " \t");
    checkBracePair(tmp_vec, brace);
    if (tmp == "") {
      continue;
    }
    if (tmp_vec[0] == "server" && tmp_vec.size() == 2) {
      if (tmp_vec[1] != "{") {
        throw FAIL;
      }
      server_blocks_.push_back(ServerBlock(file));
      brace.pop();
      error_flag |= (1 << 0);
    } else if (tmp_vec[0] == "program_name" && tmp_vec.size() == 2) {
      server_program_name_ = tmp_vec[1];
      error_flag |= (1 << 1);
    } else if (tmp_vec[0] == "http_version" && tmp_vec.size() == 2) {
      http_version_ = tmp_vec[1];
      error_flag |= (1 << 2);
    } else if (tmp_vec[0] == "status_path" && tmp_vec.size() == 2) {
      status_path_ = tmp_vec[1];
      parseStatusFile(status_path_.c_str());
      error_flag |= (1 << 3);
    } else if (tmp_vec[0] == "mime_path" && tmp_vec.size() == 2) {
      mime_path_ = tmp_vec[1];
      parseMIMEFile(mime_path_.c_str());
      error_flag |= (1 << 4);
    }
  }
  if (error_flag != 31 && !brace.empty()) {
    throw FAIL;
  }
}

void Config::parseStatusFile(const char *file_path) {
  std::fstream file;

  file.open(file_path, std::fstream::in);
  if (!file.is_open()) {
    throw FAIL;
  }
  while (!file.eof()) {
    std::string line;
    std::getline(file, line);
    std::string tmp = skipCharset(line, " \t");
    std::vector<std::string> tmp_vec = split(line, ":");
    if (tmp == "") {
      continue;
    }
    if (tmp_vec.size() == 2) {
      status_messages_[atoi(tmp_vec[0].c_str())] = tmp_vec[1];
    }
  }
}

void Config::parseMIMEFile(const char *file_path) {
  std::fstream file;

  file.open(file_path, std::fstream::in);
  if (!file.is_open()) {
    throw FAIL;
  }
  while (!file.eof()) {
    std::string line;
    std::getline(file, line);
    std::string tmp = skipCharset(line, " \t");
    std::vector<std::string> tmp_vec = split(line, " \t");
    if (tmp == "") {
      continue;
    }
    if (tmp_vec.size() != 1) {
      for (int i = 1; i < tmp_vec.size(); ++i) {
        mime_types_[tmp_vec[i]] = tmp_vec[0];
      }
    }
  }
}

std::string Config::getServerProgramName(void) const {
  return server_program_name_;
}

std::string Config::getHttpVersion(void) const {
  return http_version_;
}

std::vector<ServerBlock> Config::getServerBlocks(void) const {
  return server_blocks_;
}