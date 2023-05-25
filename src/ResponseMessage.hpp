// Copyright 2023 ean, hanbkim, jiyunpar

#include <map>
#include <string>
#include <vector>

class ResponseMessage {
 public:
  explicit ResponseMessage(int &response_status_code);
  ResponseMessage &operator=(const ResponseMessage &rhs);
  std::string generateMessage() const;

 private:
  std::string response_message_;
  ssize_t written_;
  std::string status_protocol_;
  int &response_status_code_;
  std::string status_message_;
  std::map<std::string, std::string> headers_;
  std::vector<char> body_;
};
/*
HTTP/1.1 200 OK
header ...
*/
