// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_REQUESTMESSAGE_HPP_
#define SRC_REQUESTMESSAGE_HPP_

#include <map>
#include <string>

class RequestMessage {
 public:
  RequestMessage(int &response_status_code_);
  std::string getMethod(void) const;
  void append(std::string &buf);
  int parse();
  bool writeDone();

 private:
  enum ParseState {
    kReady,
    kParse
  };
  
  ssize_t written_;
  std::string leftover_;
  std::string method_;
  std::string uri_;
  std::string protocol_;
  std::map<std::string, std::string> headers_;
  std::string body_;
  int &response_status_code_;
};

#endif  // SRC_REQUESTMESSAGE_HPP
