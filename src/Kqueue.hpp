// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_KQUEUE_HPP_
#define SRC_KQUEUE_HPP_

#include <sys/event.h>

#include <vector>

struct Kqueue {
  Kqueue(const std::vector<int> &listen_sockets);

  void setEvent(int listen_socket, int filter, int flag);

  int fd_;
  struct kevent event_register_;
  struct kevent event_list_[10];
};

#endif  // SRC_KQUEUE_HPP_