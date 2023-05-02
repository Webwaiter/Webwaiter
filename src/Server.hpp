// Copyright 2023 ean, hanbkim, jiyunpar

#ifndef SRC_SERVER_HPP_
#define SRC_SERVER_HPP_

#include <vector>

#include "src/Client.hpp"
#include "src/Config.hpp"
#include "src/Kqueue.hpp"

class Server {
 public:
  Server(const Config &config, const std::vector<int> &listen_sockets);
  void run();

 private:
  void acceptClient(std::vector<Client> &clients);
  void receiveRequestMessage(Client &client);
  void sendResponseMessage(Client &client);


  std::vector<int> listen_sockets_;
  Config config_;
  Kqueue kqueue_;
};

#endif  // SRC_SERVER_HPP_