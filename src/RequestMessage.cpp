// Copyright 2023 ean, hanbkim, jiyunpar

#include <sstream>
#include "src/RequestMessage.hpp"

RequestMessage::RequestMessage(int &response_status_code_) : response_status_code_(response_status_code_) {}

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

ReturnState RequestMessage::parse(const char *buffer) {
  std::vector<char> merge_buffer;
  std::string merge_buffer = leftover_ + buffer;
  leftover_.clear();
  size_t read_count = 0;

  if (parseStartLine(merge_buffer, read_count) == SUCCESS)
    return SUCCESS;
  /*
   * skipCrlf() startline이 들어오고 header가 들어오기 전 공백이 들어오면 400 에러를 뱉어야함. 그리고 crlf만 들어온 문자 스킵할 수 있어야 함.
   */
  if (parseHeaderLine(merge_buffer, read_count) == SUCCESS)
    return SUCCESS;
  if (parseBody() == SUCCESS)
    return SUCCESS;

  appendLeftover(merge_buffer, read_count);
  return AGAIN;
}

ReturnState RequestMessage::parseStartLine(std::string &buffer, size_t &read_count) {
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

ReturnState RequestMessage::parseMethod(std::string &buffer, size_t &read_count) {
  size_t space_pos = buffer.find(' ', read_count);

  if (space_pos == std::string::npos)
    return AGAIN;
  method_ = buffer.substr(read_count,  space_pos - read_count);
  state_ = kUri;
  read_count += space_pos + 1;
  return AGAIN;
}

ReturnState RequestMessage::parseUri(std::string &buffer, size_t &read_count) {
  size_t space_pos = buffer.find(' ', read_count);

  if (space_pos == std::string::npos)
    return AGAIN;
  uri_ = buffer.substr(read_count, space_pos - read_count);
  state_ = kProtocol;
  read_count += space_pos + 1;
  return AGAIN;
}

ReturnState RequestMessage::parseProtocol(std::string &buffer, size_t &read_count) {
  size_t crlf_pos = buffer.find("\r\n", read_count);

  if (crlf_pos == std::string::npos)
    return AGAIN;
  protocol_ = buffer.substr(read_count, crlf_pos - read_count);
  // if (protocol_ != "HTTP/1.1")
  //  return SUCCESS;
  //   make error status, parsing end
  state_ = kHeaderLine;
  read_count += crlf_pos + 2;
  return AGAIN;
}

ReturnState RequestMessage::parseHeaderLine(std::string &buffer, size_t &read_count) {
  if (buffer.find("\r\n", read_count) == std::string::npos) {
    return AGAIN;
  }

  if (buffer.find("\r\n\r\n", read_count) != std::string::npos) {
    state_ = kBodyLine;
  }

  std::string field_line;
  std::string delimiter("\r\n");
  size_t line_pos = 0;

  //Todo: field에 \r \n \0 있으면 예외 처리
  while ((line_pos = buffer.find(delimiter, read_count)) != std::string::npos) {
    field_line = buffer.substr(read_count, line_pos - read_count);

    // ":" 기준으로 스플릿 후 map에 저장
    size_t colon_pos = field_line.find(':', read_count);
    if (colon_pos != std::string::npos) {
      // Todo: field name에 공백이 있으면 예외 처리
      std::string field_name(field_line.substr(0, colon_pos));
      std::lower_bound(field_name.begin(), field_name.end());
      if (field_name.find(' ') != std::string::npos) {
        response_status_code_ = 400;
        return SUCCESS;
      }
      size_t value_pos = field_line.find_first_not_of(' ',colon_pos + 1);
      std::string field_value(field_line.substr(value_pos, field_line.length() - value_pos));
      headers_[field_name] = field_value;
    }
    read_count += line_pos + delimiter.length();
  }
  return AGAIN;
}

ReturnState RequestMessage::parseBody(std::string &buffer, size_t &read_count) {
  headers_[""]
  return AGAIN;
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
