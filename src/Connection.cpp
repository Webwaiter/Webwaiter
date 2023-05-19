// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/Connection.hpp"

#include <unistd.h>

#include <string>

#include "src/Server.hpp"

Connection::Connection(int connection_socket) : connection_socket_(connection_socket), buffer_() {}

int Connection::getConnectionSocket() const {
  return connection_socket_;
}

bool Connection::hasWorkToDo(Server &s) {
  int ret;
  switch (state_) {
    case READY:
      ret = parseRequestMessage();
      if (ret == -1)
        break;
      state_ = PARSE;
      break;
    case PARSE:
      ret = handleRequest();
      if (ret == -1)
        break;
      state_ = HANDLE;
      break;
    case HANDLE:
      ret = makeResponseMessage();
      if (ret == -1)
        break;
      state_ = RESPONSE;
      break;
    case RESPONSE:
      s.setEvent(connection_socket_, EVFILT_WRITE, EV_ADD | EV_DISABLE, NULL, NULL, NULL);
      state_ = READY;
      return false;
  }
  return true;
}

void Connection::appendBuffer(std::string &buf) {
  request_message_.append(buf); 
}

int Connection::parseRequestMessage() {
  return request_message_.parse();
}

int Connection::handleRequest() {
  if (request_message_.getMethod() == "GET") {
    response_message_ = ResponseMessage(request_message_);
    response_message_.readContent(request_message_);
  }
  return 1;
}

int Connection::makeResponseMessage() {
  response_message_.generateMessage();
  return 1;
}

void Connection::writeHandler(int fd) {
  char *buf;
  size_t size; // string.size()
  ssize_t written; 

  // matching string
  ssize_t ret = write(fd, buf + written, size - written);
  // response, request written update
  // written_ += ret;
  // disable write event
}

char *Connection::getReadBuffer() {
  return read_buffer_;
}
