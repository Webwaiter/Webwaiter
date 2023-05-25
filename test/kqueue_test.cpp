#include <sys/event.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>
#include <string>

using std::cout;
using std::endl;

extern char **environ;

int main() {
  int to_cgi[2];
  int from_cgi[2];
  if (pipe(to_cgi) < 0 || pipe(from_cgi) < 0) {  // pipe open error
    cout << "pipe error" << endl;
    return 1;
  }
  pid_t pid = fork();
  if (pid < 0) {  // fork error
    cout << "fork error" << endl;
    return 1;
  } else if (pid == 0) {  // child process (CGI)
    close(to_cgi[1]);
    if (to_cgi[0] != STDIN_FILENO) {
      if (dup2(to_cgi[0], STDIN_FILENO) != STDIN_FILENO) {
        cout << "dup2 error" << endl;
        return 1;
      }
      close(to_cgi[0]);
    }
    close(from_cgi[0]);
    if (from_cgi[1] != STDOUT_FILENO) {
      if (dup2(from_cgi[1], STDOUT_FILENO) != STDOUT_FILENO) {
        cout << "dup2 error" << endl;
        return 1;
      }
      close(from_cgi[1]);
    }
    std::string cgi_pathname = "printenv";
    char **cgi_argv = new char*[2]();
    cgi_argv[0] = const_cast<char*>(cgi_pathname.c_str());
    if (execve(cgi_pathname.c_str(), cgi_argv, environ) < 0) {
      cout << "execve error" << endl;
      return 1;
    }
  } else {  // parent process (server)
    close(to_cgi[0]);
    close(from_cgi[1]);
    int kqueue_fd = kqueue();
    struct kevent ch[3];
    int udata = 42;
    EV_SET(&ch[0], to_cgi[1], EVFILT_WRITE, EV_ADD, 0, 0, NULL);
    EV_SET(&ch[1], from_cgi[0], EVFILT_READ, EV_ADD | EV_DISABLE, 0, 0, NULL);
    EV_SET(&ch[2], pid, EVFILT_PROC, EV_ADD, NOTE_EXIT | NOTE_EXITSTATUS, 0, NULL);
    int ret = kevent(kqueue_fd, ch, 3, NULL, 0, NULL);
    if (ret == -1) {
      cout << "kevent error" << endl;
      return 1;
    }
    char buf[1024];
    struct kevent ev[3];
    while (true) {
      ret = kevent(kqueue_fd, NULL, 0, ev, 3, NULL);
      if (ret == -1) {
        cout << "kevent error" << endl;
        return 1;
      }
      for (int i = 0; i < ret; ++i) {
        if (ev[i].filter == EVFILT_WRITE) {
          ret = write(ev[i].ident, "name=ean&age=32\n", 16);
          if (ret < 0) {
            cout << "write error" << endl;
            return 1;
          }
          close(ev[i].ident);
        } else if (ev[i].filter == EVFILT_READ) {
          ret = read(ev[i].ident, buf, sizeof(buf));
          if (ret < 0) {
            cout << "read error" << endl;
            return 1;
          }
          close(ev[i].ident);
          cout << buf << endl;
          cout << "read udata: " << *reinterpret_cast<int*>(ev[i].udata) << endl;
          return 0;
        } else if (ev[i].filter == EVFILT_PROC) {
          cout << "parent output:" << endl;
          cout << "wait status: " << ev[i].data << endl;
          EV_SET(&ch[1], from_cgi[0], EVFILT_READ, EV_ENABLE, 0, 0, &udata);
          ret = kevent(kqueue_fd, &ch[1], 1, NULL, 0, NULL);
          if (ret == -1) {
            cout << "kevent error" << endl;
            return 1;
          }
        }
      }
    }
  }
  return 0;
}
