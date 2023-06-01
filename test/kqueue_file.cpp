#include <sys/event.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <string>

using std::cout;
using std::endl;

int main(int argc, char** argv) {
  if (argc != 2) {
    return 1;
  }
  int fd = open(argv[1], O_RDONLY);
  int kqueue_fd = kqueue();
  struct kevent ch[1];
  EV_SET(&ch[0], fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
  int ret = kevent(kqueue_fd, ch, 1, NULL, 0, NULL);
  if (ret == -1) {
    cout << "kevent error" << endl;
    return 1;
  }
  char buf[1000];
  struct kevent ev[1];
  while (true) {
    ret = kevent(kqueue_fd, NULL, 0, ev, 1, NULL);
    if (ret == -1) {
      cout << "kevent error" << endl;
      return 1;
    }
    for (int i = 0; i < ret; ++i) {
      if (ev[i].filter == EVFILT_READ) {
        cout << "event on" << endl;
        cout << "data: " << ev[i].data << endl;
        int read_cnt = read(ev[i].ident, buf, sizeof(buf));
        if (ev[i].flags & EV_EOF) {
          cout << "EV_EOF on" << endl;
        }
        if (read_cnt < 0) {
          cout << "read error" << endl;
          return 1;
        } else if (read_cnt == 0) {
          // close(ev[i].ident);
          // return 0;
        } else {
          write(STDOUT_FILENO, buf, read_cnt);
          cout << endl;
        }
      }
    }
  }
  return 0;
}
