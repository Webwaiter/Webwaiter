// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/RequestMessage.hpp"

#include <sstream>

#include "src/utils.hpp"

typedef std::vector<char>::iterator vector_iterator;
typedef std::map<std::string, std::string>::iterator map_iterator;
static const char kCrlf[] = {'\r', '\n'};
static const size_t kCrlfLength = 2;

static void toLower(char &c) {
  c = std::tolower(c);
}

RequestMessage::RequestMessage(int &response_status_code_)
    : response_status_code_(response_status_code_), state_(kMethod), content_length_(-1), chunk_size_(-1) {}

void RequestMessage::appendLeftover(const char *buffer, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    leftover_.push_back(buffer[i]);
  }
}

bool RequestMessage::writeDone() {
  if (body_.size() == static_cast<size_t>(written_)) {
    written_ = 0;
    return true;
  }
  return false;
}

ReturnState RequestMessage::parse(const char *buffer, size_t read) {
  if (read <= 0 && leftover_.empty())
    return AGAIN;

  appendLeftover(buffer, read);

  if (parseStartLine() == SUCCESS) {
    return SUCCESS;
  }
  if (skipCrlf() == SUCCESS) {
    return SUCCESS;
  }
  if (parseHeaderLine() == SUCCESS) {
    return SUCCESS;
  }
  if (parseContentLengthBody() == SUCCESS) {
    return SUCCESS;
  }
  if (parseChunkBody() == SUCCESS) {
    return SUCCESS;
  }
  return AGAIN;
}

ReturnState RequestMessage::parseStartLine() {
  if (parseMethod() == SUCCESS) {
    return SUCCESS;
  }
  if (parseUri() == SUCCESS) {
    return SUCCESS;
  }
  if (parseProtocol() == SUCCESS) {
    return SUCCESS;
  }
  return AGAIN;
}

ReturnState RequestMessage::parseMethod() {
  if (state_ != kMethod) {
    return AGAIN;
  }
  vector_iterator space_pos = std::find(leftover_.begin(), leftover_.end(), ' ');
  if (space_pos == leftover_.end())
    return AGAIN;

  method_.insert(method_.begin(), leftover_.begin(), space_pos);
  leftover_.erase(leftover_.begin(), space_pos + 1);
  state_ = kUri;
  return AGAIN;
}

ReturnState RequestMessage::parseUri() {
  if (state_ != kUri) {
    return AGAIN;
  }

  vector_iterator space_pos = std::find(leftover_.begin(), leftover_.end(), ' ');
  if (space_pos == leftover_.end())
    return AGAIN;

  uri_.insert(uri_.begin(), leftover_.begin(), space_pos);
  leftover_.erase(leftover_.begin(), space_pos + 1);
  // Todo: uri 분석
  state_ = kProtocol;
  return AGAIN;
}

ReturnState RequestMessage::parseProtocol() {
  if (state_ != kProtocol) {
    return AGAIN;
  }

  vector_iterator crlf_pos = std::search(leftover_.begin(), leftover_.end(), kCrlf, kCrlf + kCrlfLength);
  if (crlf_pos == leftover_.end())
    return AGAIN;

  protocol_.insert(protocol_.begin(), leftover_.begin(), crlf_pos);
  std::for_each(protocol_.begin(), protocol_.end(), toLower);
  if (protocol_ != "http/1.1") {
    state_ = kParseDone;
    response_status_code_ = 400;
    return SUCCESS;
  }
  state_ = kSkipCrlf;
  leftover_.erase(leftover_.begin(), crlf_pos + 2);
  return AGAIN;
}

ReturnState RequestMessage::skipCrlf() {
  if (state_ != kSkipCrlf)
    return AGAIN;
  /*
   * skipCrlf() startline이 들어오고 header가 들어오기 전 공백이 들어오면 400 에러를 뱉어야함. 그리고 crlf만 들어온 문자 스킵할 수 있어야 함.
   */
  vector_iterator line_pos;

  while (true) {
    line_pos = std::search(leftover_.begin(), leftover_.end(), kCrlf, kCrlf + kCrlfLength);
    if (line_pos == leftover_.end())
      return AGAIN;

    std::string field_line(leftover_.begin(), line_pos);
    if (field_line.empty()) {
      leftover_.erase(leftover_.begin(), line_pos + kCrlfLength);
      continue;
    }

    if (!isblank(field_line.front())) {
      state_ = kHeaderLine;
      return AGAIN;
    }

    response_status_code_ = 400;
    return SUCCESS;
  }
}

ReturnState RequestMessage::parseHeaderLine() {
  if (state_ != kHeaderLine)
    return AGAIN;

  vector_iterator line_pos;
  while (true) {
    line_pos = std::search(leftover_.begin(), leftover_.end(), kCrlf, kCrlf + kCrlfLength);
    if (line_pos == leftover_.end()) {
      return AGAIN;
    }

    std::string field_line(leftover_.begin(), line_pos);
    leftover_.erase(leftover_.begin(), line_pos + kCrlfLength);

    if (field_line.empty()) {
      return checkBodyType();
    }

    size_t colon_pos = field_line.find(':');
    if (colon_pos != std::string::npos) {
      std::string field_name(field_line.substr(0, colon_pos));
      if (field_name.find(' ') != std::string::npos) {
        response_status_code_ = 400;
        return SUCCESS;
      }
      std::for_each(field_name.begin(), field_name.end(), toLower);

      std::string field_value(field_line.begin() + colon_pos + 1, field_line.end());
      if (field_value.find('\0') != std::string::npos) {
        state_ = kParseDone;
        response_status_code_ = 400;
        return SUCCESS;
      }
      trim(field_value);
      headers_[field_name] = field_value;
    }
  }
}

ReturnState RequestMessage::checkBodyType() {
  map_iterator content_length = headers_.find("content-length");
  map_iterator chunked = headers_.find("transfer-encoding");

  if (content_length != headers_.end() && chunked != headers_.end()) {
    response_status_code_ = 400;
    return SUCCESS;
  }
  if (content_length != headers_.end()) {
    content_length_ = std::strtol(content_length->second.c_str(), NULL, 10);
    state_ = kContentLength;
  }
  if (chunked != headers_.end()) {
    content_length_ = 0;
    state_ = kChunkSize;
  }
  return AGAIN;
}

ReturnState RequestMessage::parseContentLengthBody() {
  if (state_ != kContentLength) {
    return AGAIN;
  }
  if (leftover_.size() < content_length_) {
    return AGAIN;
  }
  body_.insert(body_.end(), leftover_.begin(), leftover_.begin() + content_length_);
  leftover_.erase(leftover_.begin(), leftover_.begin() + content_length_);
  state_ = kParseDone;
  return SUCCESS;
}

ReturnState RequestMessage::parseChunkBody() {
  if (parseChunkSize() == SUCCESS) {
    return SUCCESS;
  }
  if (parseChunkData() == SUCCESS) {
    return SUCCESS;
  }
  if (parseTrailerField() == SUCCESS) {
    return SUCCESS;
  }
  return AGAIN;
}

ReturnState RequestMessage::parseChunkSize() {
  if (state_ != kChunkSize) {
    return AGAIN;
  }
  vector_iterator crlf_pos;

  crlf_pos = std::search(leftover_.begin(), leftover_.end(), kCrlf, kCrlf + kCrlfLength);
  if (crlf_pos == leftover_.end())
    return AGAIN;

  std::string chunk_size(leftover_.begin(), crlf_pos);
  leftover_.erase(leftover_.begin(), crlf_pos + kCrlfLength);
  if (chunk_size.empty()) {
    state_ = kParseDone;
    return SUCCESS;
  }

  char *end;
  chunk_size_ = strtol(chunk_size.c_str(), &end, 16);
  if (*end != '\0') {
    response_status_code_ = 400;
    state_ = kParseDone;
    return SUCCESS;
  }
  if (chunk_size_ == 0) {
    state_ = kTrailerField;
    return AGAIN;
  }
  state_ = kChunkData;
  return AGAIN;
}

ReturnState RequestMessage::parseChunkData() {
  if (state_ != kChunkData) {
    return AGAIN;
  }
  vector_iterator crlf_pos = std::search(leftover_.begin(), leftover_.end(), kCrlf, kCrlf + kCrlfLength);
  if (crlf_pos == leftover_.end())
    return AGAIN;
  body_.insert(body_.end(), leftover_.begin(), crlf_pos);
  size_t insertedData = crlf_pos - leftover_.begin();
  leftover_.erase(leftover_.begin(), crlf_pos + kCrlfLength);

  // body 문법이 틀렸다면
  if (insertedData != chunk_size_) {
    response_status_code_ = 400;
    return SUCCESS;
  }
  state_ = kChunkSize;
  return AGAIN;
}

ReturnState RequestMessage::parseTrailerField() {
  if (state_ != kTrailerField) {
    return AGAIN;
  }
  vector_iterator line_pos;
  while (true) {
    line_pos = std::search(leftover_.begin(), leftover_.end(), kCrlf, kCrlf + kCrlfLength);
    if (line_pos == leftover_.end()) {
      return AGAIN;
    }

    std::string field_line(leftover_.begin(), line_pos);
    leftover_.erase(leftover_.begin(), line_pos + kCrlfLength);
    if (field_line.empty()) {
      break;
    }
    size_t colon_pos = field_line.find(':');
    if (colon_pos != std::string::npos) {
      std::string field_name(field_line.substr(0, colon_pos));
      if (field_name.find(' ') != std::string::npos) {
        response_status_code_ = 400;
        state_ = kParseDone;
        return SUCCESS;
      }
      std::for_each(field_name.begin(), field_name.end(), toLower);
      std::string field_value(field_line.begin() + colon_pos + 1, field_line.end());
      trim(field_value);
      headers_[field_name] = field_value;
    }
  }
  // Todo: Remove "chunked" from Transer-Encoding;
  state_ = kParseDone;
  return SUCCESS;
}

const std::string &RequestMessage::getMethod(void) const {
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
const std::vector<char> &RequestMessage::getBody() const {
  return body_;
}
