// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_REQUESTMESSAGE_HPP_
#define SRC_REQUESTMESSAGE_HPP_

#include <map>
#include <string>
#include <vector>

#include "src/utils.hpp"

class RequestMessage {
 public:
  explicit RequestMessage(int &response_status_code_);
  void appendLeftover(const char *buffer, size_t n);
  ReturnState parse(const char *buffer, size_t read);
  bool writeDone();

  const std::string &getMethod(void) const;
  const std::string &getUri() const;
  const std::string &getAProtocol() const;
  const std::map<std::string, std::string> &getHeaders() const;
  const std::vector<char> &getBody() const;

 private:
  enum ParseState {
    kMethod,
    kUri,
    kProtocol,
    kSkipCrlf,
    kHeaderLine,
    kContentLength,
    kChunkSize,
    kChunkData,
    kTrailerField,
    kParseDone
  };

  ReturnState parseStartLine();
  ReturnState parseMethod();
  ReturnState parseUri();
  ReturnState parseProtocol();
  ReturnState skipCrlf();
  ReturnState parseHeaderLine();
  ReturnState checkBodyType();
  ReturnState parseContentLengthBody();
  ReturnState parseChunkBody();
  ReturnState parseChunkSize();
  ReturnState parseChunkData();
  ReturnState parseTrailerField();

  ParseState state_;
  ssize_t written_;

  std::vector<char> leftover_;
  ssize_t content_length_;
  ssize_t chunk_size_;

  std::string method_;
  std::string uri_;
  std::string protocol_;
  std::map<std::string, std::string> headers_;
  std::vector<char> body_;
  int &response_status_code_;
};

#endif  // SRC_REQUESTMESSAGE_HPP
