// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_SERVER_HPP_
#define SRC_SERVER_HPP_

#include <vector>

#include "src/Connection.hpp"
#include "src/Config.hpp"
#include "src/Kqueue.hpp"

class Server {
 public:
  Server(const Config &config, const std::vector<int> &listen_sockets);
  void run();

 private:
  Connection *acceptClient(int listen_socket);
  bool isListenSocketEvent(int catch_fd);

  Config config_;
  std::vector<int> listen_sockets_;
  Kqueue kqueue_;
};

#endif  // SRC_SERVER_HPP_
