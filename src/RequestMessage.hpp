// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_REQUESTMESSAGE_HPP_
#define SRC_REQUESTMESSAGE_HPP_

#include <map>
#include <string>

class RequestMessage {
 public:
  std::string getMethod(void) const;
  void appendLeftover(const std::string &buf);
  void parse(const std::string &read_buffer_);
  bool writeDone();

 private:
  size_t parseStartLine(const std::string &buffer);
  size_t parseMethod(const std::string &buffer);
  size_t parseUri(const std::string &buffer);
  size_t parseProtocol(const std::string &buffer);

  enum ParseState {
    kMethod,
    kUri,
    kProtocol,
    kHeaderLine,
    kBodyLine,
  };

  ParseState state_;
  ssize_t written_;

  std::string leftover_;
  std::string method_;
  std::string uri_;
  std::string protocol_;
  std::map<std::string, std::string> headers_;
  std::string body_;

};

#endif  // SRC_REQUESTMESSAGE_HPP