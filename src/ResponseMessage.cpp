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
  /* 
  * HTTP_VERSION sp STATUS_CODE sp [reason-phrase]
  */
  //http
  startline_header_.insert(startline_header_.end(), config_.getHttpVersion().begin(), config_.getHttpVersion().end());
  startline_header_.push_back(' ');
  
}

void ResponseMessage::createHeaderLine(const RequestMessage& request_message) {

}

void ResponseMessage::createResponseMessage(const RequestMessage& request_message) {
  createStartLine();
  createHeaderLine(request_message);
  response_message_.insert(response_message_.end(), startline_header_.begin(), startline_header_.end());
  response_message_.insert(response_message_.end(), body_.begin(), body_.end());
}

const std::vector<char> &ResponseMessage::getStartlineHeader() const {
  return startline_header_;
}
const std::vector<char> &ResponseMessage::getBody() const {
  return body_;
}
const std::vector<char> &ResponseMessage::getResponseMessage() const {
  return response_message_;
}
