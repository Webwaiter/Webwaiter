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
  } else {  // parent process (server)
    close(from_cgi[1]);
    // copy from_cgi[0] to a class member for HANDLING_DYNAMIC_PAGE_*
    close(to_cgi[0]);
    // copy to_cgi[1] to a class member for WRITING_TO_PIPE
  }
  return 0;
}
