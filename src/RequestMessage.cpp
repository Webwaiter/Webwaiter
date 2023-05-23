// Copyright 2023 ean, hanbkim, jiyunpar

#include <sstream>
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
  leftover_.clear();
  size_t read_count = 0;

  read_count += parseStartLine(merge_buffer);
  /*
   * skipCrlf() startline이 들어오고 header가 들어오기 전 공백이 들어오면 400 에러를 뱉어야함. 그리고 crlf만 들어온 문자 스킵할 수 있어야 함.
   */
  read_count += parseHeaderLine(merge_buffer);
//  parseBody();

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

size_t RequestMessage::parseHeaderLine(std::string &buffer) {
  // 구분자가 아예 없으면 바로 탈출
  size_t read_count = 0;

  if (buffer.find("\r\n") == std::string::npos) {
    return 0;
  }

  // 헤더 파싱 종료조건
  if (buffer.find("\r\n\r\n") != std::string::npos) {
    state_ = kBodyLine;
  }

  size_t pos = 0;
  std::string field_line;
  std::string delimiter("\r\n");

  while ((pos = buffer.find(delimiter)) != std::string::npos) {
    field_line = buffer.substr(0, pos);
    // ":" 기준으로 스플릿 후 map에 저장
    buffer.erase(0, pos + delimiter.length());
    read_count += pos + delimiter.length();
  }

  /*
   * 0. crlf crlf 들어왔으면 헤더 파싱 종료
   *
   * 1. crlf 기준으로 1줄
   * 2. crlf만 들어왔을 때 스킵
   * 3. ':' 기준으로 split
   * 4. ':' 가 없거나 :이전에 space가 있으면 틀린 형식
   *
   */
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
