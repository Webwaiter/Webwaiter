// Copyright 2023 ean, hanbkim, jiyunpar

#include <map>
#include <string>
#include <vector>
#include <deque>

#include "src/Config.hpp"
#include "src/Kqueue.hpp"
#include "src/ServerBlock.hpp"
#include "src/LocationBlock.hpp"
#include "src/RequestMessage.hpp"

class ResponseMessage {
 public:
  explicit ResponseMessage(int &response_status_code, const Config& config, Kqueue& kqueue);
  ResponseMessage &operator=(const ResponseMessage &rhs);
  void appendReadBufferToLeftoverBuffer(const unsigned char *read_buffer, ssize_t read);
  void parseCgiOutput(const ServerBlock &server_block);
  void createResponseMessage(const RequestMessage& request_message, const LocationBlock &location, const std::string &path);
  const std::vector<unsigned char> &getStatusLine() const;
  const std::vector<unsigned char> &getHeaderLine() const;
  const std::vector<unsigned char> &getBody() const;
  const std::map<std::string, std::string> &getHeaders() const;
  const std::vector<unsigned char> &getResponseMessage() const;
  void createBody(const std::string &path);
  void clear();

 private:
  void createStatusLine();
  void createHeaderLine(const RequestMessage& request_message, const LocationBlock &location, const std::string &path);
  void setLastModified(const RequestMessage &request_message, const LocationBlock &location);
  void setAllowed(const LocationBlock &location);
  void parseCgiBody();
  void parseCgiHeader(const ServerBlock &server_block);
  void parseField(std::string &field);
  std::string findMimeType(const std::string &path);
  int &response_status_code_;
  const Config& config_;
  Kqueue& kqueue_;
  std::map<std::string, std::string> headers_;
  std::deque<char> leftover_;
  std::vector<unsigned char> status_line_;
  std::vector<unsigned char> header_line_;
  std::vector<unsigned char> body_;
  std::vector<unsigned char> response_message_;
};
