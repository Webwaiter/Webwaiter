// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/Config.hpp"

Config::Config(const char *file) {
  parseConfig(file);
}

void Config::parseConfig(const char *file) {
  ports_.push_back(4242);
  root_ = "./docs/html";
  server_name_ = "webwaiter";
}