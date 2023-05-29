// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_CONNECTION_HPP_
#define SRC_CONNECTION_HPP_

#include <sys/event.h>

#include <vector>
#include <queue>
#include <string>

#include "src/Kqueue.hpp"
#include "src/RequestMessage.hpp"
#include "src/ResponseMessage.hpp"
#include "src/utils.hpp"

class Connection {
 public:
  Connection(int connection_socket, const Kqueue& kqueue);
  int getConnectionSocket() const;
  char *getReadBuffer();
  ReturnState work();
  bool checkReadSuccess();
  bool checkTimeOut();
  void parsingRequestMessage();
  void executeCGIProcess();
  void openStaticPage();
  void writingToPipe();
  ReturnState writeHandler(const struct kevent &event);
  ReturnState readHandler(const struct kevent &event);
  void closeConnection();

 private:
  enum State {
    PARSING_REQUEST_MESSAGE,
    HANDLING_STATIC_PAGE,
    HANDLING_DYNAMIC_PAGE_HEADER,
    HANDLING_DYNAMIC_PAGE_BODY,
    WRITING_TO_PIPE,
    WRITING_STATIC_PAGE,
    WRITING_DYNAMIC_PAGE_HEADER,
    WRITING_DYNAMIC_PAGE_BODY
  };

  int connection_socket_;
  char read_buffer_[8096];
  RequestMessage request_message_;
  ResponseMessage response_message_;
  State state_;
  Kqueue kqueue_;
  int response_status_code_;
  std::vector<int> fd_vec_;
  ssize_t read_;
  size_t read_cnt_;
  intptr_t leftover_data_;
};

#endif  // SRC_CONNECTION_HPP
