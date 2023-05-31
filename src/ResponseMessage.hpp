// Copyright 2023 ean, hanbkim, jiyunpar

#include <map>
#include <string>
#include <vector>

class ResponseMessage {
 public:
  explicit ResponseMessage(int &response_status_code, const Config& config);
  ResponseMessage &operator=(const ResponseMessage &rhs);
  std::string generateMessage() const;

 private:
  ssize_t written_;
  std::string status_protocol_;
  int &response_status_code_;
  const Config& config_;
  std::string status_message_;
  std::map<std::string, std::string> headers_;
  std::vector<char> startline_header_;
  std::vector<char> body_;
  std::vector<char> response_message_;
};
