// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/RequestMessage.hpp"

typedef std::deque<char>::iterator deque_iterator;
typedef std::map<std::string, std::string>::iterator map_iterator;

RequestMessage::RequestMessage(int &response_status_code_)
    : state_(kMethod), content_length_(0), chunk_size_(0), response_status_code_(response_status_code_) {}

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

void RequestMessage::clear() {
  state_ = kMethod;
  written_ = 0;
  content_length_ = 0;
  chunk_size_ = 0;
  method_.clear();
  uri_.clear();
  protocol_.clear();
  headers_.clear();
  body_.clear();
}

void RequestMessage::parseComplete(int response_status_code) {
  state_ = kParseComplete;
  response_status_code_ = response_status_code;
}

ReturnState RequestMessage::parse(const char *buffer, size_t read) {
  if (read == 0 && leftover_.empty()) {
    return AGAIN;
  }

  appendLeftover(buffer, read);

  parseStartLine();
  if (state_ == kSkipCrlf) {
    skipCrlf();
  }
  if (state_ == kHeaderLine) {
    parseHeaderLine();
  }
  if (state_ == kContentLength) {
    parseContentLengthBody();
  }
  parseChunkBody();

  if (state_ == kParseComplete) {
    return SUCCESS;
  }
  return AGAIN;
}

void RequestMessage::parseStartLine() {
  if (state_ == kMethod) {
    parseMethod();
  }
  if (state_ == kUri) {
    parseUri();
  }
  if (state_ == kProtocol) {
    parseProtocol();
  }
}

void RequestMessage::parseMethod() {
  deque_iterator space_pos = std::find(leftover_.begin(), leftover_.end(), ' ');
  if (space_pos == leftover_.end()) {
    return;
  }

  method_.insert(method_.begin(), leftover_.begin(), space_pos);
  leftover_.erase(leftover_.begin(), space_pos + 1);
  state_ = kUri;
}

void RequestMessage::parseUri() {
  deque_iterator space_pos = std::find(leftover_.begin(), leftover_.end(), ' ');
  if (space_pos == leftover_.end()) {
    return;
  }

  uri_.insert(uri_.begin(), leftover_.begin(), space_pos);
  leftover_.erase(leftover_.begin(), space_pos + 1);
  // Todo: uri 분석
  state_ = kProtocol;
}

void RequestMessage::parseProtocol() {
  deque_iterator crlf_pos = std::search(leftover_.begin(), leftover_.end(), kCrlf, kCrlf + kCrlfLength);
  if (crlf_pos == leftover_.end()) {
    return;
  }

  protocol_.insert(protocol_.begin(), leftover_.begin(), crlf_pos);
  leftover_.erase(leftover_.begin(), crlf_pos + kCrlfLength);

  if (protocol_ != "HTTP/1.1") {
    parseComplete(400);
    return;
  }
  state_ = kSkipCrlf;
}

void RequestMessage::skipCrlf() {
  deque_iterator line_pos;
  while (true) {
    line_pos = std::search(leftover_.begin(), leftover_.end(), kCrlf, kCrlf + kCrlfLength);
    if (line_pos == leftover_.end()) {
      return;
    }

    std::string field_line(leftover_.begin(), line_pos);
    if (field_line.empty()) {
      leftover_.erase(leftover_.begin(), line_pos + kCrlfLength);
      continue;
    }
    if (!isblank(field_line.front())) {
      state_ = kHeaderLine;
      return;
    }
    parseComplete(400);
    return;
  }
}

void RequestMessage::parseHeaderLine() {
  deque_iterator line_pos;
  while (true) {
    line_pos = std::search(leftover_.begin(), leftover_.end(), kCrlf, kCrlf + kCrlfLength);
    if (line_pos == leftover_.end()) {
      return;
    }

    std::string field_line(leftover_.begin(), line_pos);
    leftover_.erase(leftover_.begin(), line_pos + kCrlfLength);
    if (field_line.empty()) {
      checkBodyType();
      return;
    }
    parseField(field_line);
  }
}

void RequestMessage::parseField(std::string &field) {
  size_t colon_pos = field.find(':');
  if (colon_pos == std::string::npos) {
    return;
  }

  std::string field_name(field.begin(), field.begin() + colon_pos);
  if (field_name.find(' ') != std::string::npos) {
    parseComplete(400);
    return;
  }

  std::string field_value(field.begin() + colon_pos + 1, field.end());
  if (field_value.find('\0') != std::string::npos) {
    parseComplete(400);
    return;
  }

  std::for_each(field_name.begin(), field_name.end(), toLower);
  trim(field_value);
  headers_[field_name] = field_value;
}

void RequestMessage::checkBodyType() {
  map_iterator content_length = headers_.find("content-length");
  map_iterator chunked = headers_.find("transfer-encoding");

  if (content_length != headers_.end() && chunked != headers_.end()) {
    parseComplete(400);
    return;
  }
  if (content_length != headers_.end()) {
    content_length_ = std::strtol(content_length->second.c_str(), NULL, 10);
    state_ = kContentLength;
  } else if (chunked != headers_.end()) {
    state_ = kChunkSize;
  } else {
    state_ = kParseComplete;
  }
}

void RequestMessage::parseContentLengthBody() {
  if (leftover_.size() < static_cast<size_t>(content_length_)) {
    return;
  }

  body_.insert(body_.end(), leftover_.begin(), leftover_.begin() + content_length_);
  leftover_.erase(leftover_.begin(), leftover_.begin() + content_length_);
  parseComplete(200);
}

void RequestMessage::parseChunkBody() {
  if (state_ == kChunkSize) {
    parseChunkSize();
  }
  if (state_ == kChunkData) {
    parseChunkData();
  }
  if (state_ == kTrailerField) {
    parseTrailerField();
  }
}

void RequestMessage::parseChunkSize() {
  deque_iterator crlf_pos = std::search(leftover_.begin(), leftover_.end(), kCrlf, kCrlf + kCrlfLength);
  if (crlf_pos == leftover_.end()) {
    return;
  }

  std::string chunk_size(leftover_.begin(), crlf_pos);
  leftover_.erase(leftover_.begin(), crlf_pos + kCrlfLength);
  if (chunk_size.empty()) {
    parseComplete(400);
    return;
  }
  char *end;
  chunk_size_ = strtol(chunk_size.c_str(), &end, 16);
  if (*end != '\0') {
    parseComplete(400);
    return;
  }
  if (chunk_size_ == 0) {
    state_ = kTrailerField;
    return;
  }
  content_length_ += chunk_size_;
  state_ = kChunkData;
}

void RequestMessage::parseChunkData() {
  deque_iterator crlf_pos = std::search(leftover_.begin(), leftover_.end(), kCrlf, kCrlf + kCrlfLength);
  if (crlf_pos == leftover_.end()) {
    return;
  }

  size_t insertedData = crlf_pos - leftover_.begin();
  if (insertedData != static_cast<size_t>(chunk_size_)) {
    parseComplete(400);
    return;
  }

  body_.insert(body_.end(), leftover_.begin(), crlf_pos);
  leftover_.erase(leftover_.begin(), crlf_pos + kCrlfLength);
  state_ = kChunkSize;
}

void RequestMessage::parseTrailerField() {
  deque_iterator line_pos;
  while (true) {
    line_pos = std::search(leftover_.begin(), leftover_.end(), kCrlf, kCrlf + kCrlfLength);
    if (line_pos == leftover_.end()) {
      return;
    }

    std::string field_line(leftover_.begin(), line_pos);
    leftover_.erase(leftover_.begin(), line_pos + kCrlfLength);
    if (field_line.empty()) {
      removeChunkedInHeader();
      parseComplete(200);
      return;
    }
    parseField(field_line);
  }
}

void RequestMessage::removeChunkedInHeader() {
  map_iterator it = headers_.find("transfer-encoding");
  std::string &value = it->second;
  size_t pos = value.find("chunked");
  value.erase(pos, 7);

  headers_["content-length"] = numberToString(content_length_);
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
ssize_t RequestMessage::getContentLength() const {
  return content_length_;
}

std::string RequestMessage::getContentType() const {
  return headers_.count("content-type") ? headers_.at("content-type") : "";
}
const std::vector<char> &RequestMessage::getBody() const {
  return body_;
}
