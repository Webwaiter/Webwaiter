#include <sys/event.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <string>

using std::cout;
using std::endl;

int main(int argc, char** argv) {
  if (argc != 3) {
    return 1;
  }
  int fd[2];
  if (pipe(fd) == -1) {
    cout << "pipe error" << endl;
    return 1;
  }
  pid_t pid = fork();
  if (pid < 0) {
    cout << "fork error" << endl;
    return 1;
  } else if (pid == 0) {  // child
    close(fd[0]);
    if (fd[1] != STDOUT_FILENO) {
      if (dup2(fd[1], STDOUT_FILENO) != STDOUT_FILENO) {
        cout << "dup2 error" << endl;
        return 1;
      }
      close(fd[1]);
    }
    char** args = new char*[3]();
    args[0] = argv[1];
    args[1] = argv[2];
    if (execve(args[0], args, NULL) == -1) {
      cout << "execve error" << endl;
      return 1;
    }
  } else {  // parent
    close(fd[1]);
    int kqueue_fd = kqueue();
    struct kevent ch[2];
    EV_SET(&ch[0], fd[0], EVFILT_READ, EV_ADD, 0, 0, NULL);
    EV_SET(&ch[1], pid, EVFILT_PROC, EV_ADD, NOTE_EXIT, 0, NULL);
    int ret = kevent(kqueue_fd, ch, 2, NULL, 0, NULL);
    if (ret == -1) {
      cout << "kevent error" << endl;
      return 1;
    }
    char buf[1000];
    struct kevent ev[2];
    while (true) {
      ret = kevent(kqueue_fd, NULL, 0, ev, 2, NULL);
      if (ret == -1) {
        cout << "kevent error" << endl;
        return 1;
      }
      for (int i = 0; i < ret; ++i) {
        if (ev[i].filter == EVFILT_READ) {
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
        } else if (ev[i].filter == EVFILT_PROC) {
          cout << "proc event on" << endl;
          int p = waitpid(ev[i].ident, NULL, 0);
          if (p != ev[i].ident) {
            cout << "waitpid error" << endl;
          }
          // close(fd[0]);
          // return 0;
        }
      }
    }
  }
  return 0;
}
