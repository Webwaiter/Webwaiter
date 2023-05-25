// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/ResponseMessage.hpp"

#include <string>
#include <sstream>

#include "src/RequestMessage.hpp"

ResponseMessage::ResponseMessage(int &response_status_code) : response_status_code_(response_status_code) {}

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
