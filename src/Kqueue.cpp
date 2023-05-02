// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/Kqueue.hpp"

Kqueue::Kqueue(const std::vector<int> &listen_sockets) : fd_(kqueue()) {
  for (int i = 0; i < listen_sockets.size(); ++i) {
    setEvent(listen_sockets[i], EVFILT_READ, EV_ADD);
  }
}

void Kqueue::setEvent(int listen_socket, int filter, int flag) {
  EV_SET(&event_register_, listen_socket, filter, flag, NULL, NULL, NULL);
  kevent(fd_, &event_register_, 1, NULL, 0, NULL);
}