// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/Server.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <sys/event.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <deque>
#include <fstream>
#include <set>

#include "src/Config.hpp"
#include "src/Connection.hpp"
#include "src/Kqueue.hpp"
#include "src/utils.hpp"

Server::Server(const Config &config, const std::vector<int> &listen_sockets, std::ofstream &log)
    : config_(config), listen_sockets_(listen_sockets), kqueue_(), log_(log) {
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
  Connection *p = new Connection(connection_socket, kqueue_, config_);
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

static void eraseConnection(Connection *ptr, std::set<Connection*> &connections, std::deque<Connection*> &work_queue) {
  ptr->closeConnection();
  delete ptr;
  connections.erase(ptr);
  for (std::deque<Connection*>::iterator it = work_queue.begin(); it != work_queue.end(); ++it) {
    if (*it == ptr) {
      work_queue.erase(it);
      break;
    }
  }
}

void Server::run() {
  struct timespec timeout = {0, 0};
  std::set<Connection*> connections;
  std::deque<Connection*> work_queue;
  struct kevent event_list[10];
  while (1) {
    int detected_cnt = kevent(kqueue_.fd_, NULL, 0, event_list, 10, &timeout);
    for (int i = 0; i < detected_cnt; ++i) {
      int id = event_list[i].ident;
      int filter = event_list[i].filter;
      Connection *ptr = reinterpret_cast<Connection*>(event_list[i].udata);
      if (isListenSocketEvent(id)) {
        try {
          Connection *new_connection = acceptClient(id);
          log_ << "socket: " << new_connection->getConnectionSocket() << std::endl;
          connections.insert(new_connection);
          work_queue.push_back(new_connection);
        } catch (int &e) {
          log_ << "accept error: " << std::strerror(errno) << std::endl;
        }
      } else if (filter == EVFILT_READ) {
        if (ptr->readHandler(event_list[i]) == FAIL) {
          log_ << "close read" << std::endl;
          eraseConnection(ptr, connections, work_queue);
          break;
        }
        kqueue_.setEvent(id, EVFILT_READ, EV_DISABLE, 0, 0, NULL);
      } else if (filter == EVFILT_WRITE) {
        if (ptr->writeHandler(event_list[i]) == FAIL) {
          log_ << "close write" << std::endl;
          eraseConnection(ptr, connections, work_queue);
          break;
        }
        kqueue_.setEvent(id, EVFILT_WRITE, EV_DISABLE, 0, 0, NULL);
      } else if (filter == EVFILT_PROC) {   
        int ret = waitpid(id, NULL, 0);
        int status = event_list[i].data;
        ptr->clearCgiPid();
        if ((WIFEXITED(status) && WEXITSTATUS(status) != 0)
            || WIFSIGNALED(status) || ret != id) {
          ptr->setResponseStatusCode(500);
          break;
        }
      }
    }
    for (size_t queue_size = work_queue.size(); queue_size > 0; --queue_size) {
      Connection *connection = work_queue.front();
      work_queue.pop_front();
      if (connection->work() == CONNECTION_CLOSE) {
        log_ << "close: " << connection->getConnectionSocket() << std::endl;
        eraseConnection(connection, connections, work_queue);
      } else {
        work_queue.push_back(connection);
      }
    }
  }
}

const Config &Server::getConfig() const {
  return config_;
}

const Kqueue &Server::getKqueue() const {
  return kqueue_;
}
