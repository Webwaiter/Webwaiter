#include <unistd.h>

#include <iostream>
#include <string>

using std::cout;
using std::endl;

ReturnState Connection::executeCGIProcess() {
  int to_cgi[2];
  int from_cgi[2];
  if (pipe(to_cgi) < 0 || pipe(from_cgi) < 0) {
    return FAIL;
  }
  pid_t pid = fork();
  if (pid < 0) {
    return FAIL;
  }

  // child process (CGI)
  if (pid == 0) {
    // plumbing
    if (to_cgi[0] != STDIN_FILENO) {
      if (dup2(to_cgi[0], STDIN_FILENO) != STDIN_FILENO) {
        return FAIL;
      }
      close(to_cgi[0]);
    }
    if (from_cgi[1] != STDOUT_FILENO) {
      if (dup2(from_cgi[1], STDOUT_FILENO) != STDOUT_FILENO) {
        return FAIL;
      }
      close(from_cgi[1]);
    }
    for (int fd = STDERR_FILENO + 1; fd < OPEN_MAX; ++fd) {
      close(fd);
    }

    // setting up for exec
    char *cgi_pathname = server_config_.getCGIPath(url);  // Config& config
    char **cgi_argv = new char*[2]();
    cgi_argv[0] = cgi_pathname;
    char **meta_variables = new char*[kMetaVariableCount + 1]();  // kMetaVariableCount

    // exec
    if (execve(cgi_pathname, cgi_argv, meta_variables) < 0) {
      return FAIL;
    }
  }

  // parent process (server)
  close(to_cgi[0]);
  close(from_cgi[1]);
  kqueue_.setEvent(pid, EVFILT_PROC, EV_ADD, NOTE_EXIT | NOTE_EXITSTATUS, 0, this);
  kqueue_.setEvent(from_cgi[0], EVFILT_READ, EV_ADD | EV_DISABLE, NULL, 0, this);
  if (request_message_.getMethod() == "POST") {
    kqueue_.setEvent(to_cgi[1], EVFILT_WRITE, EV_ADD, NULL, 0, this);
    state_ = WRITING_TO_PIPE;
  } else {
    close(to_cgi[1]);
    kqueue_.setEvent(from_cgi[0], EVFILT_READ, EV_ENABLE, NULL, 0, this);
    state_ = HANDLING_DYNAMIC_PAGE_HEADER;
  }
  return SUCCESS;
}
