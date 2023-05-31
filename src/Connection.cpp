// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/Connection.hpp"

#include <unistd.h>

#include <queue>
#include <string>

#include "src/Server.hpp"
#include "src/utils.hpp"

Connection::Connection(int connection_socket, Kqueue& kqueue, const Config& config)
    : connection_socket_(connection_socket), file_fd_(-1), pipe_read_fd_(-1), pipe_write_fd_(-1),
      response_status_code_(200), kqueue_(kqueue), config_(config), read_(0), read_cnt_(0), leftover_data_(0),
      write_buffer_(NULL), written_(0), write_buffer_size_(0), request_message_(response_status_code_),
      response_message_(response_status_code_, config_, kqueue_), cur_server_(NULL), cur_location_(NULL), time_(time(NULL)) {}

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

ReturnState Connection::checkFileReadDone() {
  if (leftover_data_ <= sizeof(read_buffer_)) 
    if (read_ == leftover_data_) {
      // 다 읽은 상태
      close(file_fd_);
      return SUCCESS;
  }
  // 다 못읽은 상태
  kqueue_.setEvent(file_fd_, EVFILT_READ, EV_ENABLE, 0, 0, this);
  return AGAIN;
}

ReturnState Connection::handlingStaticPage() {
  response_message_.appendReadBufferToLeftoverBuffer(read_buffer_, read_);
  ReturnState ret = checkFileReadDone();
  if (ret == AGAIN) {
    return AGAIN;
  }
  response_message_.createResponseMessage(request_message_);
  // write event enable
  kqueue_.setEvent(connection_socket_, EVFILT_WRITE, EV_ENABLE, 0, 0, this);
  state_ = WRITING_STATIC_PAGE;
  //TODO: update write buffer & write buffer size
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
    case HANDLING_STATIC_PAGE:handlingStaticPage();
      break;
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
  // post의 경우 request 본문을 pipe에 쓰는 도중에 cgi 프로세스가 종료되었을 때
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
  if (file_fd_ != -1) {
    close(file_fd_);
  }
  if (pipe_read_fd_ != -1) {
    close(pipe_read_fd_);
  }
  if (pipe_write_fd_ != -1) {
    close(pipe_write_fd_);
  }
}
