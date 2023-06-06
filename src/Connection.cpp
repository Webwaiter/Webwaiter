// Copyright 2023 ean, hanbkim, jiyunpar

#include "src/Connection.hpp"

#include <unistd.h>
#include <fcntl.h>

#include <queue>
#include <set>
#include <string>

#include "src/Config.hpp"
#include "src/LocationBlock.hpp"
#include "src/Server.hpp"
#include "src/utils.hpp"

Connection::Connection(int connection_socket, Kqueue& kqueue, const Config& config)
    : connection_socket_(connection_socket), pipe_read_fd_(-1), pipe_write_fd_(-1),
      response_status_code_(200), kqueue_(kqueue), config_(config), read_(0), read_cnt_(0), leftover_data_(-1),
      write_buffer_(NULL), written_(0), write_buffer_size_(0), request_message_(response_status_code_),
      response_message_(response_status_code_, config_, kqueue_), selected_server_(NULL), selected_location_(NULL), time_(time(NULL)),
      is_connection_close_(false) {}

int Connection::getConnectionSocket() const {
  return connection_socket_;
}

void Connection::parsingRequestMessage() {
  if (request_message_.parse(read_buffer_, read_) == AGAIN) {
    kqueue_.setEvent(connection_socket_, EVFILT_READ, EV_ENABLE, 0, 0, this);
    return;
  }
  read_ = 0;
  leftover_data_ = -1;
  updateTime(time_);
  // TODO: 파싱유효성 검사
  if (response_status_code_ == 200) {
    setConfigInfo();
  }
  // TODO: allowed method 검사
  if (response_status_code_ == 200) {
    checkAllowedMethod();
  }
  std::string path = createPagePath();
  // TODO: extension 확인 후 CGI 혹은 static page 처리
  // TODO: directory listing logic 구현
  handlingStaticPage(path);
  // executeCgiProcess();
}

void Connection::checkAllowedMethod() {
  const std::string &request_method = request_message_.getMethod();
  const std::set<std::string> &allowed_method = selected_location_->getAllowedMethod();
  if (allowed_method.find(request_method) == allowed_method.end()) {
    response_status_code_ = 405;
  }
}

std::string Connection::createPagePath() {
  // status code가  200이 아니라면 default page path를 반환해야 한다.
  std::string path = "";
  std::string default_error_page = selected_server_->getDefaultErrorPages().at("400");
  if (response_status_code_ != 200) {
    if (response_status_code_ >= 300 & response_status_code_ <= 399) {
      return path;
    }
    return default_error_page;
  }

  path = selected_location_->getRootDir() + request_message_.getResourcePath();

  if (request_message_.getMethod() == "DELETE") {
    if (deleteFile(path)) {
      return path;
    } else {
      response_status_code_ = 404;
      return default_error_page;
    }
  }
  if (isDirectory(path)) {
    path += "/";
    path += selected_location_->getIndex();
    if (access(path.c_str(), R_OK | F_OK) != -1) {
      return path;
    } else {
      // return directoryListing();
    }
  } else if (access(path.c_str(), R_OK | F_OK) != -1) {
    return path;
  } 
  response_status_code_ = 404;
  return default_error_page;
}

ReturnState Connection::checkPipeReadDone() {
  if (leftover_data_ == 0) {
    close(pipe_write_fd_);
    close(pipe_read_fd_);
    pipe_read_fd_ = -1;
    pipe_write_fd_ = -1;
    return SUCCESS;
  }
  response_message_.appendReadBufferToLeftoverBuffer(read_buffer_, read_);
  read_= 0;
  // 다 못읽은 상태
  return AGAIN;
}

void Connection::handlingDynamicPage() {
  if (checkPipeReadDone() == AGAIN) {
    kqueue_.setEvent(pipe_read_fd_, EVFILT_READ, EV_ENABLE, 0, 0, this);
    return;
  }
  response_message_.parseCgiOutput(*selected_server_);
  response_message_.createResponseMessage(request_message_, *selected_location_);
  write_buffer_ = response_message_.getResponseMessage().data();
  write_buffer_size_ = response_message_.getResponseMessage().size();
  kqueue_.setEvent(connection_socket_, EVFILT_WRITE, EV_ENABLE, 0, 0, this);
  state_ = kWritingToSocket;
}

void Connection::handlingStaticPage(const std::string &path) {
  response_message_.createBody(path);
  response_message_.createResponseMessage(request_message_, *selected_location_);
  // update write buffer & write buffer size
  write_buffer_ = response_message_.getResponseMessage().data();
  write_buffer_size_ = response_message_.getResponseMessage().size();
  // write event enable & update state
  kqueue_.setEvent(connection_socket_, EVFILT_WRITE, EV_ENABLE, 0, 0, this);
  state_ = kWritingToSocket;
}

bool Connection::isTimeOut() {
  if (state_ == kReadingFromSocket && read_ == 0) {
     if (getTimeOut(time_) >= static_cast<double>(config_.getTimeout())) {
      return true;
    }
  } else {
     if (getTimeOut(time_) >= static_cast<double>(config_.getTimeout())) {
      return true;
     }
  }
  return false;
}

ReturnState Connection::work() {
  if (isTimeOut()){
    return CONNECTION_CLOSE;
  }
  switch (state_) {
    case kReadingFromSocket:
      parsingRequestMessage();
      break;
    case kWritingToPipe:
      writingToPipe();
      updateTime(time_);
      break;
    case kReadingFromPipe:
      handlingDynamicPage();
      updateTime(time_);
      break;
    case kWritingToSocket:
      writingToSocket();
      updateTime(time_);
      break;
  }
  if (is_connection_close_) {
    return CONNECTION_CLOSE;
  }
  return SUCCESS;
}

ReturnState Connection::writeHandler(const struct kevent &event) {
  // socket에 응답 메세지를 쓰는 도중 client가 강제로 커넥션을 끊었을 때
  // post의 경우 request 본문을 pipe에 쓰는 도중에 cgi 프로세스가 종료되었을 때
  if (event.flags == EV_EOF) {
    return FAIL;
  }
  ssize_t ret = write(event.ident, write_buffer_ + written_, write_buffer_size_ - written_);
  if (ret == -1) {
    return FAIL;
  }
  written_ += ret;
  // disable write event
  kqueue_.setEvent(event.ident, EVFILT_WRITE, EV_DISABLE, 0, 0, this);
  return SUCCESS;
}

ReturnState Connection::readHandler(const struct kevent &event) {
  // client가 강제로 커넥션을 끊었을 때
  // cgi프로세스가 출력을 만드는 중간에 강제로 종료되었을 때
  if (event.flags == EV_EOF) {
    return FAIL;
  }
  leftover_data_ = event.data;
  if ((read_ = read(event.ident, read_buffer_, sizeof(read_buffer_))) == -1) {
    return FAIL;
  }
  read_cnt_ += read_;
  //disable read event
  kqueue_.setEvent(event.ident, EVFILT_READ, EV_DISABLE, 0, 0, this);
  return SUCCESS;
}

char *Connection::getReadBuffer() {
  return read_buffer_;
}

void Connection::closeConnection() {
  close(connection_socket_);
  if (pipe_read_fd_ != -1) {
    close(pipe_read_fd_);
  }
  if (pipe_write_fd_ != -1) {
    close(pipe_write_fd_);
  }
}

void Connection::writingToPipe() {
  if (static_cast<size_t>(written_) < write_buffer_size_) {
    kqueue_.setEvent(pipe_write_fd_, EVFILT_WRITE, EV_ENABLE, 0, 0, this);
    return;
  }
  state_ = kReadingFromPipe;
}

void Connection::writingToSocket() {
  if (static_cast<size_t>(written_) < write_buffer_size_) {
    kqueue_.setEvent(connection_socket_, EVFILT_WRITE, EV_ENABLE, 0, 0, this);
    return;
  }
  const std::map<std::string, std::string> &response_headers = response_message_.getHeaders();
  if (response_headers.at("connection") == "close") {
    is_connection_close_ = true;
  }
  clear();
  state_ = kReadingFromSocket;
  kqueue_.setEvent(connection_socket_, EVFILT_READ, EV_ENABLE, 0, 0, this);
}

void Connection::setConfigInfo() {
  const ServerBlock &sb = config_.getServerBlocks()[0];
  const LocationBlock &lb = sb.getLocationBlocks()[0];
  selected_server_ = &sb;
  selected_location_ = &lb;
  request_message_.setResourcePath(*selected_location_);
  if (selected_location_->getRedirection() != "") {
    response_status_code_ = 301;
  }
}

static std::string getQueryString(std::string &uri) {
  size_t query_pos = uri.find('?');
  return query_pos == std::string::npos ? "" : uri.substr(query_pos + 1);
}

static std::string getScriptName(const std::string &uri, const std::string &extention) {
  size_t dot_pos = uri.find("." + extention);
  return uri.substr(0, dot_pos + extention.size() + 1);
}

char **Connection::setMetaVariables(std::map<std::string, std::string> &env) {
  env["AUTH_TYPE"] = "";
  env["REQUEST_URI"] = request_message_.getUri();
  env["QUERY_STRING"] = getQueryString(env["REQUEST_URI"]);
  env["CONTENT_LENGTH"] = numberToString(request_message_.getContentLength());
  env["CONTENT_TYPE"] = request_message_.getContentType();
  env["GATEWAY_INTERFACE"] = config_.getCgiVersion();
  env["DOCUMENT_ROOT"] = selected_location_->getRootDir();
  env["REQUEST_METHOD"] = request_message_.getMethod();
  env["SERVER_NAME"] = selected_server_->getServerName();
  env["SERVER_PORT"] = selected_server_->getServerPort();
  env["SERVER_PROTOCOL"] = config_.getHttpVersion();
  env["SERVER_SOFTWARE"] = config_.getServerProgramName();
  std::string ip = changeBinaryToIp(client_addr_.sin_addr);
  env["REMOTE_ADDR"] = ip;
  env["REMOTE_IDENT"] = "";
  env["REMOTE_USER"] = "";
  env["REMOTE_HOST"] = env["REMOTE_ADDR"];
  env["SCRIPT_NAME"] = getScriptName(env["REQUEST_URI"], selected_location_->getCgiExtension());
  env["SCRIPT_FILENAME"] = env["DOCUMENT_ROOT"] + env["SCRIPT_NAME"];
  env["PATH_INFO"] = env["SCRIPT_FILENAME"];
  env["PATH_TRANSLATED"] = env["PATH_INFO"];
  int n = env.size();
  char **meta_variables = new char*[n + 1];
  int i = 0;
  for (std::map<std::string, std::string>::iterator it = env.begin();
       it != env.end(); ++it) {
    std::string s = it->first + '=' + it->second;
    meta_variables[i] = new char[s.size() + 1];
    std::strcpy(meta_variables[i], s.c_str());
    ++i;
  }
  meta_variables[i] = 0;
  return meta_variables;
}

static char **setCgiArguments(const std::string &cgi_path, std::string &script_filename) {
  char** argv = new char*[3];
  argv[0] = new char[cgi_path.size() + 1];
  std::strcpy(argv[0], cgi_path.c_str());
  argv[1] = new char[script_filename.size() + 1];
  std::strcpy(argv[1], script_filename.c_str());
  argv[2] = 0;
  return argv;
}

void Connection::executeCgiProcess() {
  int to_cgi[2];
  int from_cgi[2];
  if (pipe(to_cgi) < 0 || pipe(from_cgi) < 0) {
    is_connection_close_ = true;
  }
  pid_t pid = fork();
  if (pid < 0) {
    is_connection_close_ = true;
  }

  // child process (CGI)
  if (pid == 0) {
    // plumbing
    if (to_cgi[0] != STDIN_FILENO) {
      if (dup2(to_cgi[0], STDIN_FILENO) != STDIN_FILENO) {
        is_connection_close_ = true;
      }
      close(to_cgi[0]);
    }
    if (from_cgi[1] != STDOUT_FILENO) {
      if (dup2(from_cgi[1], STDOUT_FILENO) != STDOUT_FILENO) {
        is_connection_close_ = true;
      }
      close(from_cgi[1]);
    }
    for (int fd = STDERR_FILENO + 1; fd < OPEN_MAX; ++fd) {
      close(fd);
    }

    // exec
    std::map<std::string, std::string> env;
    char **meta_variables = setMetaVariables(env);
    char **cgi_argv = setCgiArguments(selected_location_->getCgiPath(), env["SCRIPT_FILENAME"]);
    if (execve(cgi_argv[0], cgi_argv, meta_variables) < 0) {
      is_connection_close_ = true;
    }
  }

  // parent process (server)
  close(from_cgi[1]);
  close(to_cgi[0]);
  if (fcntl(from_cgi[0], F_SETFL, O_NONBLOCK) == -1) {
    is_connection_close_ = true;
  }
  if (fcntl(to_cgi[1], F_SETFL, O_NONBLOCK) == -1) {
    is_connection_close_ = true;
  }
  pipe_read_fd_ = from_cgi[0];
  pipe_write_fd_ = to_cgi[1];
  kqueue_.setEvent(pid, EVFILT_PROC, EV_ADD, NOTE_EXIT | NOTE_EXITSTATUS, 0, this);
  kqueue_.setEvent(pipe_read_fd_, EVFILT_READ, EV_ADD | EV_DISABLE, 0, 0, this);
  if (request_message_.getMethod() == "POST") {
    kqueue_.setEvent(pipe_write_fd_, EVFILT_WRITE, EV_ADD, 0, 0, this);
    write_buffer_ = request_message_.getBody().data();
    write_buffer_size_ = request_message_.getBody().size();
    state_ = kWritingToPipe;
  } else {
    close(pipe_write_fd_);
    pipe_write_fd_ = -1;
    state_ = kReadingFromPipe;
  }
}

void Connection::clear() {
  response_status_code_ = 200;
  read_ = 0;
  read_cnt_ = 0;
  leftover_data_ = 0;
  written_ = 0;
  write_buffer_ = NULL;
  write_buffer_size_ = 0;
  request_message_.clear();
  response_message_.clear();
}

int Connection::getPipeReadFd() const {
  return pipe_read_fd_;
}
