// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/RequestMessage.hpp"

#include <iostream>

typedef std::deque<char>::iterator deque_iterator;
typedef std::map<std::string, std::string>::iterator map_iterator;

RequestMessage::RequestMessage(int &response_status_code_)
    : state_(kStartLine), content_length_(0), chunk_size_(0), response_status_code_(response_status_code_) {}

void RequestMessage::appendLeftover(const char *buffer, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    leftover_.push_back(buffer[i]);
  }
}

void RequestMessage::clear() {
  state_ = kStartLine;
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
  if (state_ == kBetweenHeaderStart) {
    skipCrlf(kHeaderLine);
  }
  if (state_ == kHeaderLine) {
    parseHeaderLine();
  }
  if (state_ == kContentLength) {
    parseContentLengthBody();
  }
  parseChunkBody();

  if (state_ == kParseComplete) {
    validation();
    return SUCCESS;
  }
  return AGAIN;
}

void RequestMessage::printRequestMessage() {
  std::cout << method_ << "\n";
  std::cout << uri_ << "\n";
  std::cout << protocol_ << "\n";
  for (map_iterator i = headers_.begin(); headers_.end() != i; ++i) {
    std::cout << "[" << i->first << "][" << i->second << "]\n";
  }
  for (size_t i = 0; i < body_.size(); ++i) {
    std::cout << body_[i];
  }
  std::cout << std::endl;

}

void RequestMessage::parseStartLine() {
  if (state_ == kStartLine) {
    skipCrlf(kMethod);
  }
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
  if (!(method_ == "GET" || method_ == "POST" || method_ == "DELETE")) {
    parseComplete(501);
  }
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

  if (!(protocol_ == "HTTP/1.1" || protocol_ == "HTTP/1.0")) {
    parseComplete(400);
    return;
  }
  state_ = kBetweenHeaderStart;
}

void RequestMessage::skipCrlf(ParseState next_state) {
  deque_iterator line_pos;
  while (true) {
    line_pos = std::search(leftover_.begin(), leftover_.end(), kCrlf, kCrlf + kCrlfLength);
    if (line_pos == leftover_.end()) {
      return;
    }

    std::string line(leftover_.begin(), line_pos);
    if (line.empty()) {
      leftover_.erase(leftover_.begin(), line_pos + kCrlfLength);
      continue;
    }
    if (!isblank(line.front())) {
      state_ = next_state;
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
  if (!(state_ == kChunkSize || state_ == kChunkData || state_ == kTrailerField)) {
    return;
  }
  ReturnState return_state = SUCCESS;
  while (return_state == SUCCESS) {
    if (state_ == kChunkSize) {
      return_state = parseChunkSize();
    }
    if (state_ == kChunkData) {
      return_state = parseChunkData();
    }
    if (state_ == kTrailerField) {
      return_state = parseTrailerField();
    }
    if (state_ == kParseComplete) {
      return;
    }
  }
}

ReturnState RequestMessage::parseChunkSize() {
  deque_iterator crlf_pos = std::search(leftover_.begin(), leftover_.end(), kCrlf, kCrlf + kCrlfLength);
  if (crlf_pos == leftover_.end()) {
    return AGAIN;
  }

  std::string chunk_size(leftover_.begin(), crlf_pos);
  leftover_.erase(leftover_.begin(), crlf_pos + kCrlfLength);
  if (chunk_size.empty()) {
    parseComplete(400);
    return SUCCESS;
  }
  char *end;
  chunk_size_ = strtol(chunk_size.c_str(), &end, 16);
  if (*end != '\0') {
    parseComplete(400);
    return SUCCESS;
  }
  if (chunk_size_ == 0) {
    state_ = kTrailerField;
    return SUCCESS;
  }
  content_length_ += chunk_size_;
  state_ = kChunkData;
  return SUCCESS;
}

ReturnState RequestMessage::parseChunkData() {
  deque_iterator crlf_pos = std::search(leftover_.begin(), leftover_.end(), kCrlf, kCrlf + kCrlfLength);
  if (crlf_pos == leftover_.end()) {
    return AGAIN;
  }

  size_t insertedData = crlf_pos - leftover_.begin();
  if (insertedData != static_cast<size_t>(chunk_size_)) {
    parseComplete(400);
    return SUCCESS;
  }

  body_.insert(body_.end(), leftover_.begin(), crlf_pos);
  leftover_.erase(leftover_.begin(), crlf_pos + kCrlfLength);
  state_ = kChunkSize;
  return SUCCESS;
}

ReturnState RequestMessage::parseTrailerField() {
  deque_iterator line_pos;
  while (true) {
    line_pos = std::search(leftover_.begin(), leftover_.end(), kCrlf, kCrlf + kCrlfLength);
    if (line_pos == leftover_.end()) {
      return AGAIN;
    }

    std::string field_line(leftover_.begin(), line_pos);
    leftover_.erase(leftover_.begin(), line_pos + kCrlfLength);
    if (field_line.empty()) {
      removeChunkedInHeader();
      parseComplete(200);
      return SUCCESS;
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

void RequestMessage::validation() {
  if (response_status_code_ != 200) {
    return;
  }
  if (headers_.find("host") == headers_.end()) {
    response_status_code_ = 400;
    return;
  }

  map_iterator connection_pos = headers_.find("connection");
  if (connection_pos == headers_.end()) {
    return;
  }
  std::string connection = connection_pos->second;
  if (!(connection == "keep-alive" || connection == "close")) {
    response_status_code_ = 400;
  }
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

const std::string &RequestMessage::getResourcePath() const {
  return resource_path_;
}

void RequestMessage::setResourcePath(const LocationBlock &location_block) {
  std::string ret;
  const std::string &location_block_url = location_block.getUrl();
  size_t pos = uri_.find('?');
  if (pos != std::string::npos) {
    ret = uri_.substr(0, pos); 
  } else {
    ret = uri_;
  }
  pos = ret.find(location_block_url);
  resource_path_ = ret.substr(pos + location_block_url.size());
}
