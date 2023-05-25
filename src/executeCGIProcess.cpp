#include <unistd.h>

#include <iostream>
#include <string>

using std::cout;
using std::endl;

ReturnState Connection::executeCGIProcess() {
  int to_cgi[2];
  int from_cgi[2];
  if (pipe(to_cgi) < 0 || pipe(from_cgi) < 0) {  // pipe open error
    connectionClose();
    return FAIL;
  }
  pid_t pid = fork();
  if (pid < 0) {  // fork error
    connectionClose();
    return FAIL;
  } else if (pid == 0) {  // child process (CGI)
    close(to_cgi[1]);
    if (to_cgi[0] != STDIN_FILENO) {
      if (dup2(to_cgi[0], STDIN_FILENO) != STDIN_FILENO) {
        connectionClose();
        return FAIL;
      }
      close(to_cgi[0]);
    }
    close(from_cgi[0]);
    if (from_cgi[1] != STDOUT_FILENO) {
      if (dup2(from_cgi[1], STDOUT_FILENO) != STDOUT_FILENO) {
        connectionClose();
        return FAIL;
      }
      close(from_cgi[1]);
    }
    char *cgi_pathname = server_config_.getCGIPath(url);  // Config& config
    char **cgi_argv = new char*[2]();
    cgi_argv[0] = cgi_pathname;
    char **meta_variables = new char*[kMetaVariableCount + 1]();  // kMetaVariableCount
    for (int fd = STDERR_FILENO + 1; fd < OPEN_MAX; ++fd) {
      close(fd);
    }
    if (execve(cgi_pathname, cgi_argv, meta_variables) < 0) {
      connectionClose();
      return FAIL;
    }
  }
  // parent process (server)
  close(to_cgi[0]);
  close(from_cgi[1]);
  kqueue_.setEvent(from_cgi[0], EVFILT_READ, EV_ADD | EV_DISABLE, NULL, 0, this);
  if (request_message_.getMethod() == "POST") {
    state_ = WRITING_TO_PIPE;
    kqueue_.setEvent(to_cgi[1], EVFILT_WRITE, EV_ADD, NULL, 0, this);
  } else {
    close(to_cgi[1]);
    state_ = HANDLING_DYNAMIC_PAGE_HEADER;
    kqueue_.setEvent(from_cgi[0], EVFILT_READ, EV_ENABLE, NULL, 0, this);
  }
  return SUCCESS;
}
