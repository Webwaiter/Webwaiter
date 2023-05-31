// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/ResponseMessage.hpp"

#include <string>
#include <sstream>

#include "src/RequestMessage.hpp"

ResponseMessage::ResponseMessage(int &response_status_code, const Config& config, Kqueue& kqueue)
    : response_status_code_(response_status_code), config_(config), kqueue_(kqueue) {}

ReturnState ResponseMessage::checkFileReadDone(ssize_t read, size_t read_cnt, const char *read_buffer, intptr_t leftover_data) {
  appendReadBufferToLeftoverBuffer(read_buffer, read, body_);
  if (leftover_data <= sizeof(read_buffer)) {
    if (read == leftover_data) {
      // 다 읽은 상태
      // close(file_fd);
      // return SUCCESS;
  }
  // 다 못읽은 상태
  // read event enable
  // return AGAIN

}

void ResponseMessage::appendReadBufferToLeftoverBuffer(const char *read_buffer, ssize_t read, std::vector<char> &leftover_buffer) {
  for (ssize_t i = 0; i < read; ++i) {
    leftover_buffer.push_back(read_buffer[i]);
  }
}
