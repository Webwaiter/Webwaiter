// Copyright 2023 ean, hanbkim, jiyunpar

#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>

#include <cstdio>
#include <cstdlib>
#include <vector>

#include "src/Config.hpp"
#include "src/Server.hpp"

void setupListenSocket(Config &config, std::vector<int> &listen_sockets) {
  struct sockaddr_in listen_addr;
  int listen_socket;

  for (int i = 0; i < config.ports_.size(); ++i) {
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(config.ports_[i]);
    // socklen_t server_addr_len;

    listen_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (fcntl(listen_socket, F_SETFL, O_NONBLOCK) == -1) {
      std::perror("fcntl() error");
      exit(1);
    }
    if (listen_socket == -1) {
      std::perror("socket() error");
      exit(1);
    }
    if (bind(listen_socket,
             reinterpret_cast<const struct sockaddr *>(&listen_addr),
             sizeof(listen_addr)) == -1) {
      std::perror("bind() error");
      exit(1);
    }
    if (listen(listen_socket, 5) == -1) {
      std::perror("listen() error");
      exit(1);
    }
    listen_sockets.push_back(listen_socket);
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    return 1;
  }
  Config config(argv[1]);
  std::vector<int> listen_socket;
  setupListenSocket(config, listen_socket);

  Server server(config, listen_socket);
  server.run();
}