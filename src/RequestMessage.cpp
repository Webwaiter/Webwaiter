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

bool RequestMessage::writeDone() {
  if (body_.size() == written_) {
    written_ = 0;
    return true;
  }
  return false;
}
RequestMessage::RequestMessage(int &response_status_code_) : response_status_code_(response_status_code_) {}
