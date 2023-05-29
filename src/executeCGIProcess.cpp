#include <unistd.h>

#include <iostream>
#include <string>

using std::cout;
using std::endl;

char **Connection::setCgiArguments() {
}

char **Connection::setMetaVariables() {
  std::map<std::string, std::string> env;
  env["AUTH_TYPE"] = "";
  env["REQUEST_URI"] = request_message_.getUri();
  env["QUERY_STRING"] = env["REQUEST_URI"];
  env["CONTENT_LENGTH"] = request_message_.getContentLength();
  env["CONTENT_TYPE"] = request_message_.getContentType();
  env["GATEWAY_INTERFACE"] = server_config_.getCgiVersion();
  env["DOCUMENT_ROOT"] = location_->getRootDir();
  env["REQUEST_METHOD"] = request_message_.getMethod();
  env["SERVER_NAME"] = server_->getServerName();
  env["SERVER_PORT"] = server_->getServerPort();
  env["SERVER_PROTOCOL"] = server_config_.getHttpVersion();
  env["SERVER_SOFTWARE"] = server_config_.getServerProgramName();
  env["REMOTE_ADDR"] = client_.getAddress();
  env["REMOTE_IDENT"] = "";
  env["REMOTE_USER"] = "";
  env["REMOTE_HOST"] = env["REMOTE_ADDR"];
  env["SCRIPT_NAME"] = env["REQUEST_URI"];
  env["SCRIPT_FILENAME"] = env["DOCUMENT_ROOT"] + env["SCRIPT_NAME"];
  env["PATH_INFO"] = env["SCRIPT_FILENAME"];
  env["PATH_TRANSLATED"] = env["PATH_INFO"];
  int n = env.size();
  char **ret = new char*[n + 1]();
  for (std::map<std::string, std::string>::iterator it = env.begin();
       it != env.end(); ++it) {
    string s = it->first + '=' + it->second;

  }
}

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

    // exec
    char **cgi_argv = setCgiArguments();
    char **meta_variables = setMetaVariables();
    if (execve(cgi_argv[0], cgi_argv, meta_variables) < 0) {
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
