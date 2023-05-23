// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/RequestMessage.hpp"

std::string RequestMessage::getMethod(void) const {
  return (method_);
}

void RequestMessage::appendLeftover(const std::string &buf) {
  leftover_.append(buf);
}

bool RequestMessage::writeDone() {
  if (body_.size() == static_cast<size_t>(written_)) {
    written_ = 0;
    return true;
  }
  return false;
}

void RequestMessage::parse(const std::string &buffer) {
  const std::string merge_buffer = leftover_ + buffer;
  size_t read_count = 0;

  read_count += parseStartLine(merge_buffer);
//  parseHeaderLine();
//  parseBody();

  // 찌꺼기 이어붙이기
  appendLeftover(merge_buffer.c_str() + read_count);
}

size_t RequestMessage::parseStartLine(const std::string &buffer) {
  size_t read_count = 0;
  if (state_ == kMethod) {
    read_count += parseMethod(buffer);
  }
  if (state_ == kUri) {
    read_count += parseUri(buffer.c_str() + read_count);
  }
  if (state_ == kProtocol) {
    read_count += parseProtocol(buffer.c_str() + read_count);
  }
  return read_count;
}

size_t RequestMessage::parseMethod(const std::string &buffer) {
  size_t space_pos = buffer.find(' ');

  if (space_pos == std::string::npos)
    return 0;
  method_ = buffer.substr(0, space_pos);
  state_ = kUri;
  return space_pos + 1;
}

size_t RequestMessage::parseUri(const std::string &buffer) {
  size_t space_pos = buffer.find(' ');

  if (space_pos == std::string::npos)
    return 0;
  uri_ = buffer.substr(0, space_pos);
  state_ = kProtocol;
  return space_pos + 1;
}

size_t RequestMessage::parseProtocol(const std::string &buffer) {
  size_t crlf_pos = buffer.find("\r\n");

  if (crlf_pos == std::string::npos)
    return 0;
  protocol_ = buffer.substr(0, crlf_pos);
  // if (protocol_ != "HTTP/1.1")
  //   make error status, parsing end
  state_ = kHeaderLine;
  return 0;
}

const std::string &RequestMessage::getMethod1() const {
  return method_;
}
const std::string &RequestMessage::getUri() const {
  return uri_;
}
const std::string &RequestMessage::getAProtocol() const {
  return protocol_;
}
const std::map<std::string, std::string> &RequestMessage::getHeaders() const {
  return headers_;
}
RequestMessage::RequestMessage() : state_(kMethod), written_(0) {}

//size_t RequestMessage::parseHeaderLine(const std::string &buffer)
//{
//  return 0;
//}
