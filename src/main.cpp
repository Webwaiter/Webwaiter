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

static in_addr changeIpToBinary(std::string ip) {
  struct in_addr ret;

  std::vector<std::string> ip_token = split(ip, ".");
  ret.s_addr = ((atoi(ip_token[0].c_str()) << 24) | (atoi(ip_token[1].c_str()) << 16)
                | (atoi(ip_token[2].c_str()) << 8) | atoi(ip_token[3].c_str()));
  return ret;
}

static void setupListenSocket(Config &config, std::vector<int> &listen_sockets) {
  const std::vector<ServerBlock> &server_blocks = config.getServerBlocks();

  for (int i = 0; i < server_blocks.size(); ++i) {
    struct sockaddr_in listen_addr;
    memset(&listen_addr, 0, sizeof(listen_addr));
    int listen_socket;
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(server_blocks[i].getServerPort());
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
  if (argc != 2) {
    return 1;
  }
  Config config(argv[1]);
  std::vector<int> listen_socket;
  setupListenSocket(config, listen_socket);

  Server server(config, listen_socket);
  server.run();
}