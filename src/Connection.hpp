// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_CONNECTION_HPP_
#define SRC_CONNECTION_HPP_

#include <string>

#include "src/RequestMessage.hpp"
#include "src/ResponseMessage.hpp"
#include "src/Server.hpp"

class Connection {
 public:
  Connection(int connection_socket);
  int getConnectionSocket() const;
  char *getReadBuffer();
  void appendBuffer(std::string &buf);
  bool hasWorkToDo(Server &s);
  void writeHandler(int fd);

 private:
  int parseRequestMessage();
  int handleRequest();
  int makeResponseMessage();

  enum State {
    READY,
    PARSE,
    HANDLE,
    RESPONSE
  };

  int connection_socket_;
  char read_buffer_[8096];
  RequestMessage request_message_;
  ResponseMessage response_message_;
  State state_;
};

#endif  // SRC_CONNECTION_HPP
