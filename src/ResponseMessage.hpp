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
  void appendReadBufferToLeftoverBuffer(const char *read_buffer, ssize_t read);
  void parseCgiOutput();
  void createResponseMessage(const RequestMessage& request_message, const LocationBlock &location);
  const std::vector<char> &getStatusLine() const;
  const std::vector<char> &getHeaderLine() const;
  const std::vector<char> &getBody() const;
  const std::map<std::string, std::string> &getHeaders() const;
  const std::vector<char> &getResponseMessage() const;
  void createBody(const std::string &path);
  void clear();

 private:
  void createStatusLine();
  void createHeaderLine(const RequestMessage& request_message, const LocationBlock &location);
  void setLastModified(const RequestMessage &request_message, const LocationBlock &location);
  void setAllowed(const LocationBlock &location);
  void parseCgiBody();
  int &response_status_code_;
  const Config& config_;
  Kqueue& kqueue_;
  std::map<std::string, std::string> headers_;
  std::deque<char> leftover_;
  std::vector<char> status_line_;
  std::vector<char> header_line_;
  std::vector<char> body_;
  std::vector<char> response_message_;
};
