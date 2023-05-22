// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/RequestMessage.hpp"

std::string RequestMessage::getMethod(void) const {
  return (method_);
}

void RequestMessage::appendLeftover(const std::string &buf) {
  leftover_.append(buf);
}

bool RequestMessage::writeDone() {
  if (body_.size() == written_) {
    written_ = 0;
    return true;
  }
  return false;
}

void RequestMessage::parse(const std::string &buffer) {
  size_t read_count = 0;

  read_count += parseStartLine(buffer);
  parseHeaderLine();
  parseBody();

  // 찌꺼기 이어붙이기
  appendLeftover(buffer.c_str() + read_count);
}

size_t RequestMessage::parseStartLine(const std::string &buffer) {
  size_t read_count;
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
  method_.append(buffer, space_pos);
  state_ = kUri;
  return space_pos;
}

size_t RequestMessage::parseUri(const std::string &buffer) {
  size_t space_pos = buffer.find(' ');

  if (space_pos == std::string::npos)
    return 0;
  uri_.append(buffer, space_pos);
  state_ = kProtocol;
  return space_pos;
}

size_t RequestMessage::parseProtocol(const std::string &buffer) {
  size_t crlf_pos = buffer.find("\r\n");

  if (crlf_pos == std::string::npos)
    return 0;
  protocol_.append(buffer, crlf_pos);
  // if (protocol_ != "HTTP/1.1")
  //   make error status, parsing end
  state_ = kHeaderLine;
  return 0;
}
