// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/ResponseMessage.hpp"

#include <string>
#include <sstream>

#include "src/RequestMessage.hpp"

ResponseMessage::ResponseMessage(int &response_status_code, const Config& config, Kqueue& kqueue)
    : response_status_code_(response_status_code), config_(config), kqueue_(kqueue) {}


void ResponseMessage::appendReadBufferToLeftoverBuffer(const char *read_buffer, ssize_t read) {
  for (ssize_t i = 0; i < read; ++i) {
    body_.push_back(read_buffer[i]);
  }
}

void ResponseMessage::createStartLine() {

}

void ResponseMessage::createHeaderLine(const RequestMessage& request_message) {

}

void ResponseMessage::createResponseMessage(const RequestMessage& request_message) {
  createStartLine();
  createHeaderLine(request_message);
  response_message_.insert(response_message_.end(), startline_header_.begin(), startline_header_.end());
  response_message_.insert(response_message_.end(), body_.begin(), body_.end());
}
