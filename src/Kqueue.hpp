// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_KQUEUE_HPP_
#define SRC_KQUEUE_HPP_

#include <sys/event.h>

struct Kqueue {
  Kqueue() : fd_(kqueue()) {}

  void setEvent(int regist_fd, int16_t filter, uint16_t flag, uint32_t fflags, intptr_t data, void *udata) {
    struct kevent event_register;
    EV_SET(&event_register, regist_fd, filter, flag, fflags, data, udata);
    kevent(fd_, &event_register, 1, NULL, 0, NULL);
  }

  int fd_;
};

#endif  // SRC_KQUEUE_HPP_
