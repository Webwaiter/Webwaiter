#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>

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

  if (listen(server_fd, 5) == -1) {
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

  int kq = kqueue();
  struct kevent event_register;

  EV_SET(&event_register, server_fd, EVFILT_READ, EV_ADD, NULL, NULL, NULL);
  kevent(kq, &event_register, 1, NULL, 0, NULL);

  struct kevent event_list[10];
  char buf[1];
  while (1) {
    int detected_cnt = kevent(kq, NULL, 0, event_list, 10, NULL);
    for (int i = 0; i < detected_cnt; ++i) {
      if (event_list[i].ident == server_fd) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd == -1) {
          continue;
        }
        cout << "conneceted Connection :" << client_fd << '\n';
        fcntl(client_fd, F_SETFL, O_NONBLOCK);
        EV_SET(&event_register, client_fd, EVFILT_READ, EV_ADD | EV_ENABLE, NULL, NULL, NULL);
        kevent(kq, &event_register, 1, NULL, 0, NULL);
      } else if(event_list[i].filter == EVFILT_READ) {
        int rd = recv(event_list[i].ident, buf, sizeof(buf), 0);
        buf[rd] = '\0';
        cout << "1. read length : " << event_list[i].data << '\n';
        cout << "buf : " << static_cast<int>(buf[0]) << " " << '\n';
        if (rd == 0) {
          cout << "3. closed Connection :" << event_list[i].ident << '\n';
          EV_SET(&event_register, event_list[i].ident, EVFILT_WRITE, EV_ADD | EV_ENABLE, NULL, NULL, NULL);
          kevent(kq, &event_register, 1, NULL, 0, NULL);
        }
        /*
        1. 읽는다
        2. 읽은 버퍼에 \cr\lf가 있는지 확인한다
        3. 없다면 임시 변수에 붙인다
        4, 있다면 파싱한다.
        */
       /*
       get http 1.1
       a : b
       c : d
       e : f
       a :
       */
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
