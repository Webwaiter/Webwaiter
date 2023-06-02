// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/ResponseMessage.hpp"

#include <sys/stat.h>

#include <string>
#include <sstream>

#include "src/RequestMessage.hpp"
#include "src/utils.hpp"

ResponseMessage::ResponseMessage(int &response_status_code, const Config& config, Kqueue& kqueue) : response_status_code_(response_status_code), config_(config), kqueue_(kqueue) {}
void ResponseMessage::appendReadBufferToLeftoverBuffer(const char *read_buffer, ssize_t read) {
  for (ssize_t i = 0; i < read; ++i) {
    body_.push_back(read_buffer[i]);
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

std::string getCurrentHTTPDate(std::time_t *t) {
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
  headers_["Last-Modified"] = getCurrentHTTPDate(&tmp.st_mtime);
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
  headers_["Allowed"] = allowed;
}

void ResponseMessage::createHeaderLine(const RequestMessage &request_message, const LocationBlock &location) {
  //Server: in config
  //Date: 
  // If-Modified-Since
  const std::map<std::string, std::string> &request_headers = request_message.getHeaders();
  
  headers_["Server"] = config_.getServerProgramName();
  headers_["Date"] = getCurrentHTTPDate(NULL);
  if (request_headers.find("connection") != request_headers.end()) {
    headers_["Connection"] = request_headers.at("connection");
  } else {
    headers_["Connection"] = "keep-alive"; 
  }
  if (request_headers.find("if-modified-since") != request_headers.end()) {
    setLastModified(request_message, location);
  }
  if (response_status_code_ == 301) {
    headers_["Location"] = location.getRedirection();
  }
  if (response_status_code_ == 405) {
    setAllowed(location);
  }
  
  headers_["Content-Length"] = numberToString(body_.size());
  //TODO: Add logic to find MIME type
  headers_["Content-Type"] = "text/html";

  for (std::map<std::string, std::string>::iterator it = headers_.begin(); it != headers_.end(); ++it) {
    std::string field = it->first + ": " + it->second;
    appendData(header_line_, field);
    appendCrlf(header_line_);
  }
  appendCrlf(header_line_);
}

void ResponseMessage::createResponseMessage(const RequestMessage& request_message, const LocationBlock &location) {
  createStatusLine();
  createHeaderLine(request_message, location);
  response_message_.insert(response_message_.end(), status_line_.begin(), status_line_.end());
  response_message_.insert(response_message_.end(), header_line_.begin(), header_line_.end());
  response_message_.insert(response_message_.end(), body_.begin(), body_.end());
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
