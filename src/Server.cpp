// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/Server.hpp"

#include <vector>

#include "src/Client.hpp"
#include "src/Config.hpp"

Server::Server(const Config &config, const std::vector<int> &listen_sockets) : listen_sockets_(listen_sockets), kqueue_(listen_sockets) {
  //config_ = config;
}

void Server::run() {
  int kq = kqueue();
  struct kevent event_register;

  EV_SET(&event_register, server_fd, EVFILT_READ, EV_ADD, NULL, NULL, NULL);
  kevent(kq, &event_register, 1, NULL, 0, NULL);

  struct kevent event_list[10];
  char buf[10000];
  while (1) {
    int detected_cnt = kevent(kq, NULL, 0, event_list, 10, NULL);
    for (int i = 0; i < detected_cnt; ++i) {
      if (event_list[i].ident == server_fd) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd == -1) {
          continue;
        }
        cout << "conneceted client :" << client_fd << '\n';
        fcntl(client_fd, F_SETFL, O_NONBLOCK);
        EV_SET(&event_register, client_fd, EVFILT_READ, EV_ADD | EV_ENABLE, NULL, NULL, NULL);
        kevent(kq, &event_register, 1, NULL, 0, NULL);
      } else if(event_list[i].filter == EVFILT_READ) {
        int rd = recv(event_list[i].ident, buf, event_list[i].data, 0);
        buf[rd] = '\0';
        cout << "1. read length : " << event_list[i].data << '\n';
        if (rd <= 0) {
          cout << "3. closed client :" << event_list[i].ident << '\n';
        }
        EV_SET(&event_register, event_list[i].ident, EVFILT_WRITE, EV_ADD | EV_ENABLE, NULL, NULL, NULL);
        kevent(kq, &event_register, 1, NULL, 0, NULL);
      } else if(event_list[i].filter == EVFILT_WRITE) {
        cout << "2. write" << '\n';
        cout << buf << '\n';
        int fd = open("simple.html", O_RDONLY);
        int rd = read (fd, buf, 10000);
        buf[rd] = '\0';
        if (send(event_list[i].ident, buf, strlen(buf), 0) == -1)
          cout << "fail" << '\n';
        EV_SET(&event_register, event_list[i].ident, EVFILT_WRITE, EV_DELETE, NULL, NULL, NULL);
        kevent(kq, &event_register, 1, NULL, 0, NULL);
      }
    }
  }
}

void Server::acceptClient(std::vector<Client> &clients) {

}

void Server::receiveRequestMessage(Client &client) {

}

void Server::sendResponseMessage(Client &client) {

}