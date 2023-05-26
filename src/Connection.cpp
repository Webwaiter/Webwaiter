// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/Connection.hpp"

#include <unistd.h>

#include <queue>
#include <string>

#include "src/Server.hpp"
#include "src/utils.hpp"

Connection::Connection(int connection_socket, const Kqueue& kqueue)
    : connection_socket_(connection_socket), request_message_(response_status_code_), response_message_(response_status_code_), kqueue_(kqueue),
      response_status_code_(200) {}

int Connection::getConnectionSocket() const {
  return connection_socket_;
}

void Connection::parsingRequestMessage() {
  /*
  // 1. 파싱
  request_message_.parse();

  
  if (isCGIExtension) {
    executeCGIProcess();
  } else {
    openStaticPage();
  }
  */
}

void Connection::writingToPipe() {
  if (request_message_.writeDone()) {
    state_ = HANDLING_DYNAMIC_PAGE_HEADER;
  }
}

ReturnState Connection::work(void) {
  if (checkReadSuccess() == false) {
    if (checkTimeOut()){
      // connectionClose();
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

ReturnState Connection::writeHandler(const struct kevent &event) {
  char *buf = 0;
  size_t size = 0; // string.size()
  ssize_t written = 0; 

  // matching string
  ssize_t ret = write(fd, buf + written, size - written);
  (void)ret;
  // response, request written update
  // written_ += ret;
  // disable write event
}

ReturnState Connection::readHandler(const struct kevent &event) {
  // client가 강제로 커넥션을 끊었을 때
  // cgi프로세스가 출력을 만드는 중간에 강제로 종료되었을 때
  if (event.flags == EV_EOF) {
    return FAIL;
  }
  int left = event.data;
  if (left <= sizeof(read_buffer_)) {
    if ((read_ = read(event.ident, read_buffer_, sizeof(read_buffer_))) == -1) {
      return FAIL;
    }
    if (read_ == left) {
      read_cnt_ += read_;
      close(event.ident);
    }
  } else {
    if ((read_ = read(event.ident, read_buffer_, sizeof(read_buffer_))) == -1) {
      return FAIL;
    }
    read_cnt_ += read_;
  }
}

char *Connection::getReadBuffer() {
  return read_buffer_;
}

void Connection::closeConnection() {
  close(connection_socket_);
  for (size_t i = 0; i < fd_vec_.size(); ++i) {
    close(fd_vec_[i]);
  }
}
