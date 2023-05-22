// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_CONNECTION_HPP_
#define SRC_CONNECTION_HPP_

#include <queue>
#include <string>

#include "src/RequestMessage.hpp"
#include "src/ResponseMessage.hpp"
#include "src/ReturnState.hpp"
#include "src/Server.hpp"

class Connection {
 public:
  Connection(int connection_socket);
  int getConnectionSocket() const;
  char *getReadBuffer();
  ReturnState work();
  bool checkReadSuccess();
  bool checkTimeOut();
  void parsingRequestMessage();
  void executeCGIProcess();
  void openStaticPage();
  void writingToPipe();
  void writeHandler(int fd);

 private:
  enum State {
    PARSING_REQUEST_MESSAGE,
    HANDLING_STATIC_PAGE,
    HANDLING_DYNAMIC_PAGE_HEADER,
    HANDLING_DYNAMIC_PAGE_BODY,
    WRITING_TO_PIPE,
    WRITING_STATIC_PAGE,
    WRITING_DYNAMIC_PAGE_HEADER,
    WRITING_DYNAMIC_PAGE_BODY,
  };

  int connection_socket_;
  char read_buffer_[8096];
  RequestMessage request_message_;
  ResponseMessage response_message_;
  State state_;
};

#endif  // SRC_CONNECTION_HPP