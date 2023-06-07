// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/Config.hpp"

#include "src/utils.hpp"

Config::Config(const char *file_path) {
  try {
    parseConfigFile(file_path);
    checkSemantics();
  } catch(ReturnState) {
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

static void checkHttpVersion(std::string http_version) {
  if (http_version != "HTTP/1.1") {
    throw FAIL;
  }
}

static void checkCgiVersion(std::string cgi_version) {
  if (cgi_version != "CGI/1.1") {
    throw FAIL;
  }
}

static void checkStatusPath(std::string status_path) {
  if (access(status_path.c_str(), R_OK | F_OK) == -1) {
    throw FAIL;
  }
}

static void checkMimePath(std::string mime_path) {
  if (access(mime_path.c_str(), R_OK | F_OK) == -1) {
    throw FAIL;
  }
}

static void checkTimeout(int timeout) {
  if (timeout <= 0 || timeout >= 100) {
    throw FAIL;
  }
}

static void checkDefaultErrorPage(std::string default_error_page) {
  if (access(default_error_page.c_str(), R_OK | F_OK) == -1) {
    throw FAIL;
  }
}

void Config::checkSemantics() const {
  checkHttpVersion(http_version_);
  checkCgiVersion(cgi_version_);
  checkStatusPath(status_path_);
  checkMimePath(mime_path_);
  checkTimeout(timeout_);
  checkDefaultErrorPage(default_error_page_);
}

void Config::parseConfigFile(const char *file_path) {
  std::ifstream file;
  int error_flag = 0;
  std::stack<std::string> brace;

  file.open(file_path);
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
      parseMimeFile(mime_path_.c_str());
      error_flag |= (1 << 4);
    } else if (tmp_vec[0] == "timeout" && tmp_vec.size() == 2) {
      for (size_t i = 0; i < tmp_vec[1].size(); ++i) {
        if (!isdigit(tmp_vec[1][i])) {
          throw FAIL;
        }
      }
      timeout_ = atoi(tmp_vec[1].c_str());
      error_flag |= (1 << 5);
    } else if (tmp_vec[0] == "cgi_version" && tmp_vec.size() == 2) {
      cgi_version_ = tmp_vec[1];
      error_flag |= (1 << 6);
    } else if (tmp_vec[0] == "default_error_page" && tmp_vec.size() == 2) {
      default_error_page_ = tmp_vec[1];
      error_flag |= (1 << 7);
    }
  }
  if (!(error_flag == 255) || !brace.empty()) {
    throw FAIL;
  }
}

void Config::parseStatusFile(const char *file_path) {
  std::ifstream file;

  file.open(file_path);
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
      status_messages_[tmp_vec[0]] = tmp_vec[1];
    }
  }
}

void Config::parseMimeFile(const char *file_path) {
  std::ifstream file;

  file.open(file_path);
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
      for (size_t i = 1; i < tmp_vec.size(); ++i) {
        mime_types_[tmp_vec[i]] = tmp_vec[0];
      }
    }
  }
}

const std::string &Config::getServerProgramName() const {
  return server_program_name_;
}

const std::string &Config::getHttpVersion() const {
  return http_version_;
}

const std::string &Config::getCgiVersion() const {
  return cgi_version_;
}

const std::vector<ServerBlock> &Config::getServerBlocks() const {
  return server_blocks_;
}

const int &Config::getTimeout() const {
  return timeout_;
}

const std::string &Config::getDefaultErrorPage() const {
  return default_error_page_;
}

const std::map<std::string, std::string> &Config::getStausMessages() const {
  return status_messages_;
}

const std::map<std::string, std::string> &Config::getMimeTypes() const {
  return mime_types_;
}
