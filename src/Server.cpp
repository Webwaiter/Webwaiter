// Copyright 2023 ean, hanbkim, jiyunpar

#include <fcntl.h>
#include <unistd.h>
#include <sys/event.h>

#include <cstdio>
#include <map>
#include <queue>

#include "src/ReturnState.hpp"
#include "src/Server.hpp"

Server::Server(const Config &config, const std::vector<int> &listen_sockets)
    : config_(config), listen_sockets_(listen_sockets) {
  // config_ = config;
  for (size_t i = 0; i < listen_sockets.size(); ++i) {
    kqueue_.setEvent(listen_sockets[i], EVFILT_READ, EV_ADD, 0, NULL, NULL);
  }
}

const Connection& Server::acceptClient(int listen_socket, std::vector<Connection>& v) {
  int connection_socket = accept(listen_socket, NULL, NULL);
  if (connection_socket == -1) {
    throw 1;
  }
  fcntl(connection_socket, F_SETFL, O_NONBLOCK);
  kqueue_.setEvent(connection_socket, EVFILT_READ, EV_ADD, 0, NULL, NULL);
  Connection c(connection_socket, kqueue_);
  v.push_back(c);
  return v.back();
}

// void Server::receiveRequestMessage(Connection &connection) { 
//   int fd = connection.getConnectionSocket();
//   int rd = read(fd, buf, kBufferSize);
//   if (rd <= 0) {
//     return;
//   }
//   buf[rd] = '\0';

//   connection.appendBuffer(buf);
// }

// void Server::sendResponseMessage(Connection &connection) {
//   connection.writeHandler();
// }

bool Server::isListenSocketEvent(int catch_fd) {
  for (size_t i = 0; i < listen_sockets_.size(); ++i) {
    if (catch_fd == listen_sockets_[i])
      return true;
  }
  return false;
}

void Server::run() {
  std::vector<Connection> connections;
  std::map<int, Connection&> connection_map;
  std::queue<Connection*> work_queue;

  struct kevent event_list[10];
  while (1) {
    int detected_cnt = kevent(kqueue_.fd_, NULL, 0, event_list, 10, NULL);
    for (int i = 0; i < detected_cnt; ++i) {
      int fd = event_list[i].ident;
      int filter = event_list[i].filter;

      if (isListenSocketEvent(fd)) {
        try {
          connections[fd] = acceptClient(fd, connections);
          work_queue.push(&connections[fd]);
        } catch (int &e) {
          std::perror("accept() error");
        }
      } else if(filter == EVFILT_READ) {
        char *buf = connections[fd].getReadBuffer();
        read(fd, buf, sizeof(buf));
        kqueue_.setEvent(fd, EVFILT_READ, EV_DISABLE, 0, 0, NULL);
      } else if(filter == EVFILT_WRITE) {
        connections[fd].writeHandler(fd);
        kqueue_.setEvent(fd, EVFILT_WRITE, EV_DISABLE, 0, 0, NULL);
      }
    }
    for (size_t queue_size = work_queue.size(); queue_size > 0; --queue_size) {
      Connection &connection = *(work_queue.front());
      work_queue.pop();
      if (connection.work() == CONNECTION_CLOSE) {
        //pop fd - connection pair from map
      } else {
        work_queue.push(&connection);
      }
    }
  }
}
