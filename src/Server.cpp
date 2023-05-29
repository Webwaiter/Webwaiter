// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/Server.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <sys/event.h>

#include <cstdio>
#include <queue>
#include <set>

#include "src/Config.hpp"
#include "src/Connection.hpp"
#include "src/Kqueue.hpp"
#include "src/utils.hpp"

Server::Server(const Config &config, const std::vector<int> &listen_sockets)
    : config_(config), listen_sockets_(listen_sockets) {
  for (size_t i = 0; i < listen_sockets.size(); ++i) {
    kqueue_.setEvent(listen_sockets[i], EVFILT_READ, EV_ADD, 0, 0, NULL);
  }
}

Connection *Server::acceptClient(int listen_socket) {
  int connection_socket = accept(listen_socket, NULL, NULL);
  if (connection_socket == -1) {
    throw 1;
  }
  fcntl(connection_socket, F_SETFL, O_NONBLOCK);
  Connection *p = new Connection(connection_socket, kqueue_);
  kqueue_.setEvent(connection_socket, EVFILT_READ, EV_ADD, 0, 0, p);
  kqueue_.setEvent(connection_socket, EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, p);
  return p;
}

bool Server::isListenSocketEvent(int catch_fd) {
  for (size_t i = 0; i < listen_sockets_.size(); ++i) {
    if (catch_fd == listen_sockets_[i]) {
      return true;
    }
  }
  return false;
}

void Server::run() {
  std::set<Connection*> connections;
  std::queue<Connection*> work_queue;
  struct kevent event_list[10];
  while (1) {
    int detected_cnt = kevent(kqueue_.fd_, NULL, 0, event_list, 10, NULL);
    for (int i = 0; i < detected_cnt; ++i) {
      int id = event_list[i].ident;
      int filter = event_list[i].filter;
      Connection *ptr = reinterpret_cast<Connection*>(event_list[i].udata);
      if (isListenSocketEvent(id)) {
        try {
          Connection *new_connection = acceptClient(id);
          connections.insert(new_connection);
          work_queue.push(new_connection);
        } catch (int &e) {
          std::perror("accept() error");
        }
      } else if (filter == EVFILT_READ) {
        ptr->readHandler(id);
        kqueue_.setEvent(id, EVFILT_READ, EV_DISABLE, 0, 0, NULL);
      } else if (filter == EVFILT_WRITE) {
        ptr->writeHandler(id);
        kqueue_.setEvent(id, EVFILT_WRITE, EV_DISABLE, 0, 0, NULL);
      } else if (filter == EVFILT_PROC) {
        int status = event_list[i].data;
        if ((WIFEXITED(status) && WEXITSTATUS(status) != 0)
            || WIFSIGNALED(status)) {
          ptr->closeConnection();
          delete ptr;
          connections.erase(ptr);
        }
      }
    }
    for (size_t queue_size = work_queue.size(); queue_size > 0; --queue_size) {
      Connection *connection = work_queue.front();
      work_queue.pop();
      if (connection->work() == CONNECTION_CLOSE) {
        //pop fd - connection pair from map
      } else {
        work_queue.push(connection);
      }
    }
  }
}
