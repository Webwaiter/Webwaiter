// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/RequestMessage.hpp"

std::string RequestMessage::getMethod(void) const {
  return (method_);
}

void RequestMessage::append(std::string &buf) {
  buffer_ += buf;
}

int RequestMessage::parse() {
  method_ = "GET";
  uri_ = "/index.html";
  protocol_ = "HTTP/1.1";

  headers_["HOST"] = "www.example.com";
  body_ = "";
  return 1;
}