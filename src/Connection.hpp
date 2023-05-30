// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_CONNECTION_HPP_
#define SRC_CONNECTION_HPP_


#include <sys/event.h>
#include <sys/time.h>
#include <netinet/in.h>

#include <vector>
#include <queue>
#include <string>

#include "src/Config.hpp"
#include "src/ServerBlock.hpp"
#include "src/LocationBlock.hpp"
#include "src/Kqueue.hpp"
#include "src/RequestMessage.hpp"
#include "src/ResponseMessage.hpp"
#include "src/utils.hpp"

class Connection {
 public:
  Connection(int connection_socket, Kqueue& kqueue, const Config& config);
  int getConnectionSocket() const;
  char *getReadBuffer();
  ReturnState work();
  bool checkReadSuccess();
  bool checkTimeOut();
  void setConfigInfo();
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
  std::vector<int> fd_vec_;
  int response_status_code_;

  Kqueue &kqueue_;
  const Config &config_;

  char read_buffer_[8096];
  ssize_t read_;
  size_t read_cnt_;
  intptr_t leftover_data_;
  
  char *write_buffer_;
  ssize_t written_;
  size_t write_buffer_size_;

  RequestMessage request_message_;
  ResponseMessage response_message_;

  ServerBlock *cur_server_;
  LocationBlock *cur_location_;

  struct sockaddr_in client_addr_;
  struct timeval time_;
  State state_;
};

#endif  // SRC_CONNECTION_HPP
