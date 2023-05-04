// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_CONNECTION_HPP_
#define SRC_CONNECTION_HPP_

#include <string>

#include "src/RequestMessage.hpp"
#include "src/Server.hpp"

class Connection {
 public:
  Connection(int connection_socket);
  int getConnectionSocket();
  void appendBuffer(std::string &buf);
  bool hasWorkToDo(Server &s);

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

  RequestMessage request_message_;
  int connection_socket_;
  std::string response_message_;
  State state_;
};

#endif  // SRC_CONNECTION_HPP
