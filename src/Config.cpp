// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/Config.hpp"
#include "src/ReturnState.hpp"

Config::Config(const char *file_path) {
  try {
    parseConfigFile(file_path);
  }
  catch(int) {
    throw FAIL;
  }
}

void Config::parseConfigFile(const char *file_path) {
  std::fstream file;

  file.open(file_path, std::ios_base::in);
  if (!file.is_open()) {
    throw FAIL;
  }
  while (!file.eof()) {
    std::string line;
    std::getline(file, line);
    if (line.find())
  }
}