// Copyright 2023 ean, hanbkim, jiyunpar

#include <map>
#include <string>
#include <vector>

#include "src/Config.hpp"
#include "src/Kqueue.hpp"
#include "src/RequestMessage.hpp"

class ResponseMessage {
 public:
  explicit ResponseMessage(int &response_status_code, const Config& config, Kqueue& kqueue);
  ResponseMessage &operator=(const ResponseMessage &rhs);
  void appendReadBufferToLeftoverBuffer(const char *read_buffer, ssize_t read);
  void createResponseMessage(const RequestMessage& request_message);

 private:
  void createStartLine();
  void createHeaderLine(const RequestMessage& request_message);
  int &response_status_code_;
  const Config& config_;
  Kqueue& kqueue_;
  std::map<std::string, std::string> headers_;
  std::vector<char> startline_header_;
  std::vector<char> body_;
  std::vector<char> response_message_;
};
