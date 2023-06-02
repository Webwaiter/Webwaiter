// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_REQUESTMESSAGE_HPP_
#define SRC_REQUESTMESSAGE_HPP_

#include <map>
#include <string>
#include <deque>

#include "src/utils.hpp"

class RequestMessage {
 public:
  explicit RequestMessage(int &response_status_code_);
  void appendLeftover(const char *buffer, size_t n);
  ReturnState parse(const char *buffer, size_t read);
  bool writeDone();
  void clear();

  const std::string &getMethod(void) const;
  const std::string &getUri() const;
  const std::string &getAProtocol() const;
  const std::map<std::string, std::string> &getHeaders() const;
  ssize_t getContentLength() const;
  std::string getContentType() const;
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
    kParseComplete
  };

  void parseStartLine();
  void parseMethod();
  void parseUri();
  void parseProtocol();
  void skipCrlf();
  void parseHeaderLine();
  void parseContentLengthBody();
  void parseChunkBody();
  void parseChunkSize();
  void parseChunkData();
  void parseTrailerField();

  void parseComplete(int response_status_code);
  void checkBodyType();
  void parseField(std::string &field);
  void removeChunkedInHeader();

  ParseState state_;
  ssize_t written_;

  std::deque<char> leftover_;
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
