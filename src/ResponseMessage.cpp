// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/ResponseMessage.hpp"

#include <sys/stat.h>

#include <string>
#include <sstream>
#include <algorithm>

#include "src/RequestMessage.hpp"
#include "src/utils.hpp"

typedef std::deque<char>::iterator deque_iterator;

ResponseMessage::ResponseMessage(int &response_status_code, const Config& config, Kqueue& kqueue) : response_status_code_(response_status_code), config_(config), kqueue_(kqueue) {}
void ResponseMessage::appendReadBufferToLeftoverBuffer(const char *read_buffer, ssize_t read) {
  for (ssize_t i = 0; i < read; ++i) {
    leftover_.push_back(read_buffer[i]);
  }
}

void ResponseMessage::parseCgiBody() {
  body_.insert(body_.end(), leftover_.begin(), leftover_.end());
}

void ResponseMessage::parseCgiHeader(const ServerBlock &server_block) {
 // headers_에 적절한 header field와 value를 파싱해서 넣는 함수
  deque_iterator line_pos;
  while (true) {
    line_pos = std::search(leftover_.begin(), leftover_.end(), kNl, kNl + kNlLength);
    if (line_pos == leftover_.end()) {
      return;
    }

    std::string field_line(leftover_.begin(), line_pos);
    leftover_.erase(leftover_.begin(), line_pos + kNlLength);
    if (field_line.empty()) {
      return;
    }
    parseField(field_line);
  }
  if (headers_.find("status") != headers_.end()) {
    response_status_code_ = std::atoi(headers_.at("status").c_str());
  }
  if (headers_.find("loaction") != headers_.end()) {
    response_status_code_ = 302;
    std::string uri = headers_.at("location");
    if (uri[0] == '/') {
      std::string absolute_uri("http://");
      std::string server_ip = server_block.getServerIP();
      std::string server_port = server_block.getServerPort();
      appendData(absolute_uri, server_ip); 
      absolute_uri.push_back(':');
      appendData(absolute_uri, server_port); 
      appendData(absolute_uri, uri);
      headers_["location"] = absolute_uri;
    }
  }
}

void ResponseMessage::parseField(std::string &field) {
  size_t colon_pos = field.find(':');
  if (colon_pos == std::string::npos) {
    return;
  }

  std::string field_name(field.begin(), field.begin() + colon_pos);
  if (field_name.find(' ') != std::string::npos) {
    response_status_code_ = 400;
    return;
  }

  std::string field_value(field.begin() + colon_pos + 1, field.end());
  if (field_value.find('\0') != std::string::npos) {
    response_status_code_ = 400;
    return;
  }

  std::for_each(field_name.begin(), field_name.end(), toLower);
  trim(field_value);
  headers_[field_name] = field_value;
}

void ResponseMessage::parseCgiOutput(const ServerBlock &server_block) {
  parseCgiHeader(server_block);
  parseCgiBody();
  if (body_.size() == 0) {
    createBody(config_.getDefaultErrorPage());  
  }
}

void ResponseMessage::createBody(const std::string &path) {
  std::ifstream file(path.c_str());
  char c;
  while ((c = file.get()) != -1 ) {
    body_.push_back(c);
  }
}

void ResponseMessage::createStatusLine() {
  appendData(status_line_, config_.getHttpVersion());
  status_line_.push_back(' ');
  
  std::string status_code = numberToString(response_status_code_);
  appendData(status_line_, status_code);
  status_line_.push_back(' ');

  appendData(status_line_, config_.getStausMessages().at(status_code));
  appendCrlf(status_line_);
}

std::string getHTTPDate(std::time_t *t) {
  std::time_t current_time;
  if (t == NULL) {
   current_time = std::time(NULL);
  } else {
    current_time = *t;
  }
  std::tm* timeInfo = std::gmtime(&current_time);

  char buffer[128];
  std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timeInfo);

  return buffer;
}

void ResponseMessage::setLastModified(const RequestMessage &request_message, const LocationBlock &location) {
  std::string path = location.getRootDir() + request_message.getUri();
  struct stat tmp;

  if (stat(path.c_str(), &tmp) == -1) {
    return;
  }
  headers_["last-modified"] = getHTTPDate(&tmp.st_mtime);
}

void ResponseMessage::setAllowed(const LocationBlock &location) {
  std::set<std::string> allowed_set = location.getAllowedMethod();

  std::string allowed;
  for (std::set<std::string>::iterator it = allowed_set.begin(); it != allowed_set.end(); ++it) {
   allowed += *it;
   if (it != (--allowed_set.end())) {
    allowed += ", ";
   }
  }
  headers_["allowed"] = allowed;
}

std::string ResponseMessage::findMimeType(const std::string &path) {
  size_t pos = path.find_last_of('.');
  if (pos == std::string::npos) {
    return "text/plain";
  }
  std::string extension = path.substr(pos + 1);
  const std::map<std::string, std::string> &mime_type = config_.getMimeTypes();
  if (mime_type.find(extension) == mime_type.end()) {
    return "text/plain";
  }
  return mime_type.at(extension);
}

void ResponseMessage::createHeaderLine(const RequestMessage &request_message, const LocationBlock &location, const std::string &path) {
  const std::map<std::string, std::string> &request_headers = request_message.getHeaders();
  
  headers_["server"] = config_.getServerProgramName();
  headers_["date"] = getHTTPDate(NULL);
  if (request_headers.find("connection") != request_headers.end()) {
    headers_["connection"] = request_headers.at("connection");
  } else {
    headers_["connection"] = "keep-alive"; 
  }
  if (request_headers.find("if-modified-since") != request_headers.end()) {
    setLastModified(request_message, location);
  }
  if (response_status_code_ == 301) {
    headers_["location"] = location.getRedirection();
  }
  if (response_status_code_ == 405) {
    setAllowed(location);
  }
   
  if (body_.size() != 0) {
    headers_["content-length"] = numberToString(body_.size());
    if (headers_.find("content-type") == headers_.end())
      headers_["content-type"] = findMimeType(path);
  }

  for (std::map<std::string, std::string>::iterator it = headers_.begin(); it != headers_.end(); ++it) {
    std::string field = it->first + ": " + it->second;
    appendData(header_line_, field);
    appendCrlf(header_line_);
  }
  appendCrlf(header_line_);
}

void ResponseMessage::createResponseMessage(const RequestMessage& request_message, const LocationBlock &location, const std::string &path) {
  createStatusLine();
  createHeaderLine(request_message, location, path);
  response_message_.insert(response_message_.end(), status_line_.begin(), status_line_.end());
  response_message_.insert(response_message_.end(), header_line_.begin(), header_line_.end());
  response_message_.insert(response_message_.end(), body_.begin(), body_.end());
}

void ResponseMessage::clear() {
  headers_.clear();
  leftover_.clear();
  status_line_.clear();
  header_line_.clear();
  body_.clear();
  response_message_.clear();
}

const std::vector<char> &ResponseMessage::getStatusLine() const {
  return status_line_;
}

const std::vector<char> &ResponseMessage::getHeaderLine() const {
  return header_line_;
}

const std::vector<char> &ResponseMessage::getBody() const {
  return body_;
}

const std::map<std::string, std::string> &ResponseMessage::getHeaders() const {
  return headers_;
}

const std::vector<char> &ResponseMessage::getResponseMessage() const {
  return response_message_;
}
