#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>

#include <cstdlib>
#include <exception>
#include <iostream>

using std::cout;
using std::endl;

int handShake(int port, struct sockaddr_in &server_addr, int &server_fd) {
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  socklen_t server_addr_len;

  server_fd = socket(PF_INET, SOCK_STREAM, 0);
  fcntl(server_fd, F_SETFL, O_NONBLOCK);
  cout << "server fd : " << server_fd << '\n';
  if (server_fd == -1) {
    return 0;
  }

  if (bind(server_fd, reinterpret_cast<const struct sockaddr *>(&server_addr), sizeof(server_addr)) == -1) {
    return 0;
  }

  if (listen(server_fd, 1) == -1) {
    return 0;
  }
  return 1;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    return 1;
  }

  struct sockaddr_in server_addr = sockaddr_in();
  int server_fd;
  handShake(std::atoi(argv[1]), server_addr, server_fd);

  int fd_max = server_fd;

  fd_set read_fds;
  fd_set write_fds;

  FD_ZERO(&read_fds);
  FD_ZERO(&write_fds);
  FD_SET(server_fd, &read_fds);
  FD_SET(server_fd, &write_fds);

  fd_set read_copy_fds;
  fd_set write_copy_fds;

  struct timeval timeout = timeval();
  timeout.tv_sec = 5;

  int available_fd_count = 0;
  while (1) {
    errno = 0;
    read_copy_fds = read_fds;
    write_copy_fds = write_fds;

    available_fd_count = select(fd_max + 1, &read_copy_fds, &write_copy_fds, NULL, &timeout);
    if (errno == EBADF || errno == ENOTSOCK) { 
      cout << available_fd_count << '\n';
      cout << errno << " continue\n";
      continue;
    }
    if (available_fd_count == 0) {
      cout << "fds 0 continue\n";
      continue;
    }

    for (int i = 0; i < fd_max + 1; ++i) {
      if (FD_ISSET(i, &read_copy_fds)) {
        if (i == server_fd) {
          int client_fd = accept(server_fd, NULL, NULL);
          cout << "accept" << client_fd << endl;
          if (client_fd == -1) {
            continue;
          }
          fcntl(client_fd, F_SETFL, O_NONBLOCK);
          FD_SET(client_fd, &read_fds);
          FD_SET(client_fd, &write_fds);
          if (fd_max < client_fd) {
            fd_max = client_fd;
          }
          cout << "connected clinet " << client_fd << endl;
        } else {
          char buf[10000];
          int rd = recv(i, buf, sizeof(buf), 0);
          cout << "rd : " << rd << '\n';
          if (rd <= 0) {
            cout << i << '\n';
            FD_CLR(i, &read_fds);
            // close(i);
          } else
            send(i, buf, rd, 0);
          // else {
          //   buf[rd] = '\0';
          //   char htmlbuf[100000];
          //   int htmlfd = open("simple.html", O_RDONLY);
          //   int htmlrd = read(htmlfd, htmlbuf, 100000);
          //   send(i, htmlbuf, htmlrd, 0);
          // }
          
        }
      }
    }
  }
  return 0;
}