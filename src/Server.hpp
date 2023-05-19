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
  void setEvent(int regist_fd, int16_t filter, uint16_t flag, uint32_t fflags, intptr_t data, void *udata);

 private:
  Connection Server::acceptClient(int listen_socket);
  void receiveRequestMessage(Connection &Connection);
  void sendResponseMessage(Connection &Connection);

  bool isListenSocketEvent(int catch_fd);
  void sendStaticMessage(void);


  std::vector<int> listen_sockets_;
  Config config_;
  Kqueue kqueue_;
};

#endif  // SRC_SERVER_HPP_
