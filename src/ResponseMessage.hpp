// Copyright 2023 ean, hanbkim, jiyunpar

#include <map>
#include <string>
#include <vector>

#include "src/Config.hpp"
#include "src/Kqueue.hpp"

class ResponseMessage {
 public:
  explicit ResponseMessage(int &response_status_code, const Config& config, Kqueue& kqueue);
  ResponseMessage &operator=(const ResponseMessage &rhs);
  ReturnState checkFileReadDone(ssize_t read, size_t read_cnt, const char *read_buffer, intptr_t leftover_data);

 private:
  void appendReadBufferToLeftoverBuffer(const char *read_buffer, ssize_t read, std::vector<char> &leftover_buffer);
  ssize_t written_;
  std::string status_protocol_;
  int &response_status_code_;
  const Config& config_;
  Kqueue& kqueue_;
  std::string status_message_;
  std::map<std::string, std::string> headers_;
  std::vector<char> startline_header_;
  std::vector<char> body_;
  std::vector<char> response_message_;
};
