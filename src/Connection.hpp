// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_CONNECTION_HPP_
#define SRC_CONNECTION_HPP_


#include <sys/event.h>
#include <netinet/in.h>

#include <vector>
#include <queue>
#include <string>
#include <ctime>

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
  void handlingStaticPage();
  void writingToSocket();
  ReturnState executeCgiProcess();
  void openStaticPage();
  void writingToPipe();
  ReturnState writeHandler(const struct kevent &event);
  ReturnState readHandler(const struct kevent &event);
  void closeConnection();

 private:
  enum State {
    kReadingFromSocket,
    kWritingToPipe,
    kReadingFromPipe,
    kWritingToSocket
  };

  ReturnState checkFileReadDone();
  char **setMetaVariables(std::map<std::string, std::string> &env);

  int connection_socket_;
  int file_fd_;
  int pipe_read_fd_;
  int pipe_write_fd_;
  int response_status_code_;

  Kqueue &kqueue_;
  const Config &config_;

  char read_buffer_[8096];
  ssize_t read_;
  size_t read_cnt_;
  intptr_t leftover_data_;
  
  const char *write_buffer_;
  ssize_t written_;
  size_t write_buffer_size_;

  RequestMessage request_message_;
  ResponseMessage response_message_;

  const ServerBlock *selected_server_;
  const LocationBlock *selected_location_;

  struct sockaddr_in client_addr_;
  time_t time_;
  bool is_connection_close_;
  State state_;
};

#endif  // SRC_CONNECTION_HPP
