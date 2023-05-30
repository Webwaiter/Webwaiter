// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/Connection.hpp"

#include <unistd.h>

#include <queue>
#include <string>

#include "src/Server.hpp"
#include "src/utils.hpp"

Connection::Connection(int connection_socket, Kqueue& kqueue, const Config& config)
    : connection_socket_(connection_socket), response_status_code_(200), kqueue_(kqueue), config_(config),
      read_(0), read_cnt_(0), leftover_data_(0), write_buffer_(NULL), written_(0), write_buffer_size_(0),
      request_message_(response_status_code_), response_message_(response_status_code_, config_), cur_server_(NULL),
      cur_location_(NULL) {}

int Connection::getConnectionSocket() const {
  return connection_socket_;
}

void Connection::parsingRequestMessage() {
  /*
  // 1. 파싱
  ReturnState ret = request_message_.parse(read_buffer_);
  if (ret == AGAIN) {
    return;
  }

  if (isCGIExtension()) {
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
    case PARSING_REQUEST_MESSAGE:parsingRequestMessage();
      break;
    case HANDLING_STATIC_PAGE:break;
    case HANDLING_DYNAMIC_PAGE_HEADER:break;
    case HANDLING_DYNAMIC_PAGE_BODY:break;
    case WRITING_TO_PIPE:writingToPipe();
      break;
    case WRITING_STATIC_PAGE:break;
    case WRITING_DYNAMIC_PAGE_HEADER:break;
    case WRITING_DYNAMIC_PAGE_BODY:break;
  }
  return SUCCESS;
}

ReturnState Connection::writeHandler(const struct kevent &event) {
  // socket에 응답 메세지를 쓰는 도중 client가 강제로 커넥션을 끊었을 때
  // pipe에 쓰는 도중에 cgi 프로세스가 종료되었을 때
  if (event.flags == EV_EOF) {
    return FAIL;
  }
  ssize_t ret = write(event.ident, write_buffer_ + written_, write_buffer_size_ - written_);
  if (ret == -1) {
    return FAIL;
  }
  written_ += ret;
  // disable write event
  kqueue_.setEvent(event.ident, EVFILT_WRITE, EV_DISABLE, 0, 0, this);
  return SUCCESS;
}

ReturnState Connection::readHandler(const struct kevent &event) {
  // client가 강제로 커넥션을 끊었을 때
  // cgi프로세스가 출력을 만드는 중간에 강제로 종료되었을 때
  if (event.flags == EV_EOF) {
    return FAIL;
  }
  leftover_data_ = event.data;
  if ((read_ = read(event.ident, read_buffer_, sizeof(read_buffer_))) == -1) {
    return FAIL;
  }
  read_cnt_ += read_;
  //disable read event
  kqueue_.setEvent(event.ident, EVFILT_READ, EV_DISABLE, 0, 0, this);
  return SUCCESS;
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
