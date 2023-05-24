#include <unistd.h>

#include <iostream>
#include <string>

using std::cout;
using std::endl;

void Connection::executeCGIProcess() {
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
    char *cgi_pathname = config.getCGIPath();  // Config& config
    char **cgi_argv = new char*[kCGIArgc + 1]();  // kCGIArgc
    cgi_argv[0] = cgi_pathname;
    char **meta_variables = new char*[kMetaVariableCount + 1]();  // kMetaVariableCount
    // close all unused file descriptors
    // if (execve(cgi_pathname, cgi_argv, meta_variables) < 0) {
    //   cout << "execve error" << endl;
    //   return 1;
    // }
  }
  // parent process (server)
  close(to_cgi[0]);
  close(from_cgi[1]);
  server.setEvent(from_cgi[0], EVFILT_READ, EV_ADD | EV_DISABLE, NULL, 0, NULL);  // PROBLEM!!!!!
  if (request_message.getMethod() == "POST") {
    state_ = WRITING_TO_PIPE;
    // prepare things to write to CGI
    server.setEvent(to_cgi[1], EVFILT_WRITE, EV_ADD, NULL, 0, NULL);  // PROBLEM!!!!!
  } else {
    close(to_cgi[1]);
    state_ = HANDLING_DYNAMIC_PAGE_HEADER;
    server.setEvent(from_cgi[0], EVFILT_READ, EV_ENABLE, NULL, 0, NULL);  // PROBLEM!!!!!
  }
  return 0;
}
