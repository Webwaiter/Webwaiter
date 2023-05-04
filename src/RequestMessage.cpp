// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/RequestMessage.hpp"

void RequestMessage::append(std::string &buf) {
  buffer_ += buf;
}

int RequestMessage::parse() {
  return 0;
}