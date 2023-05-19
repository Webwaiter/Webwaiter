// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/ResponseMessage.hpp"

#include <string>
#include <sstream>

#include "src/RequestMessage.hpp"

typedef typename std::map<std::string, std::string>::const_iterator const_iterator;

ResponseMessage::ResponseMessage(const RequestMessage &request_message) {
  status_code_ = 200;
  status_message_ = "OK";
  status_protocol_ = "HTTP/1.1";
  headers_["Content-Type"] = "text/html";
  headers_["Content-Length"] = "614";
}

std::string ResponseMessage::generateMessage() const {
  std::stringstream ss;
  ss << status_protocol_ << " " 
    << status_code_ << " "
    << status_message_ << "\r\n";
  for (const_iterator it = headers_.begin(); it != headers_.end(); ++it) {
    ss << it->first << ": " << it->second << "\r\n";
  }
  ss << "\r\n";
  ss << body_;
}