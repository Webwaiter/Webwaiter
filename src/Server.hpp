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
  const Connection& acceptClient(int listen_socket, std::vector<Connection>& v);
  void receiveRequestMessage(Connection &Connection);
  void sendResponseMessage(Connection &Connection);

  bool isListenSocketEvent(int catch_fd);
  void sendStaticMessage(void);


  Config config_;
  std::vector<int> listen_sockets_;
  Kqueue kqueue_;
};

#endif  // SRC_SERVER_HPP_
