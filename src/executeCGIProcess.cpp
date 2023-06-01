#include <arpa/inet.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>

using std::cout;
using std::endl;

static std::string getQueryString(std::string &uri) {
  size_t query_pos = uri.find('?');
  return query_pos == std::string::npos ? "" : uri.substr(query_pos + 1);
}

static std::string getScriptName(std::string &uri, std::string &extention) {
  size_t dot_pos = uri.find("." + extention);
  return uri.substr(0, dot_pos + extention.size() + 1);
}

char **Connection::setMetaVariables(std::map<std::string, std::string> &env) {
  env["AUTH_TYPE"] = "";
  env["REQUEST_URI"] = request_message_.getUri();
  env["QUERY_STRING"] = getQueryString(env["REQUEST_URI"]);
  env["CONTENT_LENGTH"] = request_message_.getContentLength();
  env["CONTENT_TYPE"] = request_message_.getContentType();
  env["GATEWAY_INTERFACE"] = config_.getCgiVersion();
  env["DOCUMENT_ROOT"] = cur_location_->getRootDir();
  env["REQUEST_METHOD"] = request_message_.getMethod();
  env["SERVER_NAME"] = cur_server_->getServerName();
  env["SERVER_PORT"] = cur_server_->getServerPort();
  env["SERVER_PROTOCOL"] = config_.getHttpVersion();
  env["SERVER_SOFTWARE"] = config_.getServerProgramName();
  char ip[INET_ADDRSTRLEN];
  inet_pton(AF_INET, &client_addr_, ip, INET_ADDRSTRLEN);
  env["REMOTE_ADDR"] = ip;
  env["REMOTE_IDENT"] = "";
  env["REMOTE_USER"] = "";
  env["REMOTE_HOST"] = env["REMOTE_ADDR"];
  env["SCRIPT_NAME"] = getScriptName(env["REQUEST_URI"], cur_location_->getCgiExtention());
  env["SCRIPT_FILENAME"] = env["DOCUMENT_ROOT"] + env["SCRIPT_NAME"];
  env["PATH_INFO"] = env["SCRIPT_FILENAME"];
  env["PATH_TRANSLATED"] = env["PATH_INFO"];
  int n = env.size();
  char **meta_variables = new char*[n + 1];
  int i = 0;
  for (std::map<std::string, std::string>::iterator it = env.begin();
       it != env.end(); ++it) {
    string s = it->first + '=' + it->second;
    meta_variables[i] = new char[s.size() + 1];
    std::strcpy(meta_variables[i], s.c_str());
    ++i;
  }
  meta_variables[i] = 0;
  return meta_variables;
}

static char **setCgiArguments(std::string &cgi_path, std::string &script_filename) {
  char** argv = new char*[3];
  argv[0] = new char[cgi_path.size() + 1];
  std::strcpy(argv[0], cgi_path.c_str());
  argv[1] = new char[script_filename.size() + 1];
  std::strcpy(argv[1], script_filename.c_str());
  argv[2] = 0;
  return argv;
}

ReturnState Connection::executeCgiProcess() {
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
    std::map<std::string, std::string> env;
    char **meta_variables = setMetaVariables(env);
    char **cgi_argv = setCgiArguments(cur_location_->getCgiPath(), env["SCRIPT_FILENAME"]);
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
