// Copyright 2023 ean, hanbkim, jiyunpar

#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>

#include <cstdio>
#include <cstdlib>
#include <vector>

#include "src/Config.hpp"
#include "src/Server.hpp"
#include "src/utils.hpp"

static void setupListenSocket(Config &config, std::vector<int> &listen_sockets) {
  const std::vector<ServerBlock> &server_blocks = config.getServerBlocks();

  for (size_t i = 0; i < server_blocks.size(); ++i) {
    struct sockaddr_in listen_addr;
    memset(&listen_addr, 0, sizeof(listen_addr));
    int listen_socket;
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(atoi(server_blocks[i].getServerPort().c_str()));
    listen_addr.sin_addr = changeIpToBinary(server_blocks[i].getServerIP());

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
      if (errno == EADDRINUSE) {
        continue;
      }
      else {
        std::perror("bind() error");
        exit(1);
      }
    }
    if (listen(listen_socket, 5) == -1) {
      std::perror("listen() error");
      exit(1);
    }
    listen_sockets.push_back(listen_socket);
  }
}

int main(int argc, char *argv[]) {
  if (!(argc == 2 || argc == 1)) {
    return 1;
  }
  try {
    Config config(argv[1]);
    std::vector<int> listen_socket;
    setupListenSocket(config, listen_socket);

    Server server(config, listen_socket);
    server.run();
  } catch (ReturnState) {
    std::cout << "error in config file\n";
    return 1;
  }
  return 0;
}
