// Copyright 2023 ean, hanbkim, jiyunpar

#include <fcntl.h>
#include <sys/event.h>

#include <cstdio>
#include <map>
#include <queue>

#include "src/Server.hpp"

Server::Server(const Config &config, const std::vector<int> &listen_sockets) : listen_sockets_(listen_sockets), kqueue_() {
  //config_ = config;
  kqueue_.fd_ = kqueue();
  for (int i = 0; i < listen_sockets.size(); ++i) {
    setEvent(listen_sockets[i], EVFILT_READ, EV_ADD, NULL, NULL, NULL);
  }
}

void Server::setEvent(int regist_fd, int16_t filter, uint16_t flag, uint32_t fflags, intptr_t data, void *udata) {
  EV_SET(&kqueue_.event_register_, regist_fd, filter, flag, NULL, NULL, NULL);
  kevent(kqueue_.fd_, &kqueue_.event_register_, 1, NULL, 0, NULL);
}

void Server::acceptClient(std::map<int, Connection> &connections, int listen_socket) {
  int connection_socket = accept(listen_socket, NULL, NULL);
  if (connection_socket == -1) {
    std::perror("accept() error");
    return;
  }
  connections[connection_socket] = Connection(connection_socket);
  fcntl(connection_socket, F_SETFL, O_NONBLOCK);
  setEvent(connection_socket, EVFILT_READ, EV_ADD, NULL, NULL);
}

void Server::receiveRequestMessage(Connection &Connection) { 
  static const int kBufferSize = 8192;
  char buf[kBufferSize];
  int fd = Connection.getConnectionSocket();
  int rd = recv(fd, buf, kBufferSize, 0);
  if (rd <= 0) {
    return;
  }
  buf[rd] = '\0';

  Connection.appendBuffer(buf);
}

void Server::sendResponseMessage(Connection &Connection) {
  if (isCGI()) {
    // cgi.runCGI();
    runCGI();
  } else {
    sendStaticMessage();
  }
}

bool Server::isListenSocketEvent(int catch_fd) {
  for (int i = 0; i < listen_sockets_.size(); ++i) {
    if (catch_fd == listen_sockets_[i])
      return true;
  }
  return false;
}

void Server::run() {
  std::map<int, Connection> connections;
  std::queue<Connection&> work_queue;

  while (1) {
    int detected_cnt = kevent(kqueue_.fd_, NULL, 0, kqueue_.event_list_, 10, NULL);
    for (int i = 0; i < detected_cnt; ++i) {
      int fd = kqueue_.event_list_[i].ident;
      int filter = kqueue_.event_list_[i].filter;

      if (isListenSocketEvent(fd)) {
        acceptClient(connections, fd);
      } else if(filter == EVFILT_READ) {
        receiveRequestMessage(connections[fd]);
        work_queue.push(connections[fd]);
      } else if(filter == EVFILT_WRITE) {
        sendResponseMessage(connections[fd]);
      }
    }
    size_t queue_size = work_queue.size();
    while (queue_size) {
      Connection& c = work_queue.front();
      if (c.hasWorkToDo(*this)) {
        work_queue.push(c);
      }
      work_queue.pop();
      --queue_size;
    }
  }
}
