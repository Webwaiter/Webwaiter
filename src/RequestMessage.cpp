// Copyright 2023 ean, hanbkim, jiyunpar

#include <sstream>
#include "src/RequestMessage.hpp"

std::string RequestMessage::getMethod(void) const {
  return (method_);
}

void RequestMessage::appendLeftover(const std::string &buffer, size_t read_count) {
  leftover_.append(buffer, read_count);
}

bool RequestMessage::writeDone() {
  if (body_.size() == static_cast<size_t>(written_)) {
    written_ = 0;
    return true;
  }
  return false;
}

void RequestMessage::parse(const std::string &buffer) {
  std::string merge_buffer = leftover_ + buffer;
  leftover_.clear();
  size_t read_count = 0;

  parseStartLine(merge_buffer, read_count);
  /*
   * skipCrlf() startline이 들어오고 header가 들어오기 전 공백이 들어오면 400 에러를 뱉어야함. 그리고 crlf만 들어온 문자 스킵할 수 있어야 함.
   */
  parseHeaderLine(merge_buffer, read_count);
//  parseBody();

  appendLeftover(merge_buffer, read_count);
}

void RequestMessage::parseStartLine(std::string &buffer, size_t &read_count) {
  if (state_ == kMethod) {
    parseMethod(buffer, read_count);
  }
  if (state_ == kUri) {
    parseUri(buffer, read_count);
  }
  if (state_ == kProtocol) {
    parseProtocol(buffer, read_count);
  }
}

void RequestMessage::parseMethod(std::string &buffer, size_t &read_count) {
  size_t space_pos = buffer.find(' ', read_count);

  if (space_pos == std::string::npos)
    return;
  method_ = buffer.substr(read_count,  space_pos - read_count);
  state_ = kUri;
  read_count += space_pos + 1;
}

void RequestMessage::parseUri(std::string &buffer, size_t &read_count) {
  size_t space_pos = buffer.find(' ', read_count);

  if (space_pos == std::string::npos)
    return;
  uri_ = buffer.substr(read_count, space_pos - read_count);
  state_ = kProtocol;
  read_count += space_pos + 1;
}

void RequestMessage::parseProtocol(std::string &buffer, size_t &read_count) {
  size_t crlf_pos = buffer.find("\r\n", read_count);

  if (crlf_pos == std::string::npos)
    return;
  protocol_ = buffer.substr(read_count, crlf_pos - read_count);
  // if (protocol_ != "HTTP/1.1")
  //   make error status, parsing end
  state_ = kHeaderLine;
  read_count += crlf_pos + 2;
}

void RequestMessage::parseHeaderLine(std::string &buffer, size_t &read_count) {
  // 구분자가 아예 없으면 바로 탈출
  if (buffer.find("\r\n", read_count) == std::string::npos) {
    return;
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
  // 헤더 파싱 종료조건
  if (buffer.find("\r\n\r\n", read_count) != std::string::npos) {
    state_ = kBodyLine;
  }

  std::string field_line;
  std::string delimiter("\r\n");
  size_t line_pos = 0;

  while ((line_pos = buffer.find(delimiter, read_count)) != std::string::npos) {
    field_line = buffer.substr(read_count, line_pos - read_count);
    // ":" 기준으로 스플릿 후 map에 저장
    size_t colon_pos = field_line.find(':', read_count);
    if (colon_pos != std::string::npos) {
      // Todo: field name에 공백이 있으면 예외 처리
      std::string field_name(field_line.substr(0, colon_pos));
      size_t value_pos = field_line.find_first_not_of(' ',colon_pos + 1);
      std::string field_value(field_line.substr(value_pos, field_line.length() - value_pos));
      headers_[field_name] = field_value;
    }
    read_count += line_pos + delimiter.length();
  }
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
