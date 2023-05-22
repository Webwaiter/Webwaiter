// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/Connection.hpp"

#include <unistd.h>

#include <queue>
#include <string>

#include "src/ReturnState.hpp"
#include "src/Server.hpp"

Connection::Connection(int connection_socket) : connection_socket_(connection_socket), read_buffer_() {}

int Connection::getConnectionSocket() const {
  return connection_socket_;
}

void Connection::parsingRequestMessage() {
  // 1. 파싱
  request_message_.parse();

  
  if (isCGIExtension) {
    executeCGIProcess();
  } else {
    openStaticPage();
  }
  
}

void Connection::writingToPipe() {
  if (request_message_.writeDone()) {
    state_ = HANDLING_DYNAMIC_PAGE_HEADER;
  }
}

ReturnState Connection::work(void) {
  if (checkReadSuccess() == false) {
    if (checkTimeOut()){
      connectionClose();
      return CONNECTION_CLOSE;
    }
  }
  switch (state_) {
    case PARSING_REQUEST_MESSAGE:
      parsingRequestMessage();
      break;
    case HANDLING_STATIC_PAGE:
      break;
    case HANDLING_DYNAMIC_PAGE_HEADER:
      break;
    case HANDLING_DYNAMIC_PAGE_BODY:
      break;
    case WRITING_TO_PIPE:
      writingToPipe();
      break;
    case WRITING_STATIC_PAGE:
      break;
    case WRITING_DYNAMIC_PAGE_HEADER:
      break;
    case WRITING_DYNAMIC_PAGE_BODY:
      break;
  }
  return SUCCESS;
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
