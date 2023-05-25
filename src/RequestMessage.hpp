// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_REQUESTMESSAGE_HPP_
#define SRC_REQUESTMESSAGE_HPP_

#include <map>
#include <string>

class RequestMessage {
 public:
  RequestMessage(int &response_status_code_);
  std::string getMethod(void) const;
  void appendLeftover(const std::string &buffer, size_t read_count);
  void parse(const std::string &read_buffer_);
  bool writeDone();

 private:
  void parseStartLine(std::string &buffer, size_t &read_count);
  void parseMethod(std::string &buffer, size_t &read_count);
  void parseUri(std::string &buffer, size_t &read_count);
  void parseProtocol(std::string &buffer, size_t &read_count);
  void parseHeaderLine(std::string &buffer, size_t &read_count);

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
 public:
  const std::string &getMethod1() const;
  const std::string &getUri() const;
  const std::string &getAProtocol() const;
  const std::map<std::string, std::string> &getHeaders() const;
 private:
  std::string method_;
  std::string uri_;
  std::string protocol_;
  std::map<std::string, std::string> headers_;
  std::string body_;
  int &response_status_code_;
};

#endif  // SRC_REQUESTMESSAGE_HPP
