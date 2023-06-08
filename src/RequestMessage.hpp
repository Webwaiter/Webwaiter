// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_REQUESTMESSAGE_HPP_
#define SRC_REQUESTMESSAGE_HPP_

#include <map>
#include <string>
#include <deque>

#include "src/utils.hpp"
#include "src/LocationBlock.hpp"
#include "ServerBlock.hpp"

class RequestMessage {
 public:
  explicit RequestMessage(int &response_status_code_);
  ReturnState parse(const unsigned char *buffer, size_t read);
  void checkOverMaxClientBodySize(const ServerBlock *server_block);
  void clear();

  const std::string &getMethod(void) const;
  const std::string &getUri() const;
  const std::string &getAProtocol() const;
  const std::map<std::string, std::string> &getHeaders() const;
  ssize_t getContentLength() const;
  std::string getContentType() const;
  const std::string &getResourcePath() const;
  void setResourcePath(const LocationBlock &location_block);
  const std::vector<unsigned char> &getBody() const;
  void printRequestMessage();

 private:
  enum ParseState {
    kStartLine,
    kMethod,
    kUri,
    kProtocol,
    kBetweenHeaderStart,
    kHeaderLine,
    kContentLength,
    kChunkSize,
    kChunkData,
    kTrailerField,
    kParseComplete
  };

  void appendLeftover(const unsigned char *buffer, size_t n);
  void parseStartLine();
  void parseMethod();
  void parseUri();
  void parseProtocol();
  void skipCrlf(ParseState next_state);
  void parseHeaderLine();
  void parseContentLengthBody();
  void parseChunkBody();
  ReturnState parseChunkSize();
  ReturnState parseChunkData();
  ReturnState parseTrailerField();

  void parseComplete(int response_status_code);
  void validation();
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
  std::string resource_path_;
  std::map<std::string, std::string> headers_;
  std::vector<unsigned char> body_;
  int &response_status_code_;
};

#endif  // SRC_REQUESTMESSAGE_HPP
