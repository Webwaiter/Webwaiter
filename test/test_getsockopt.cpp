#include <sys/socket.h>

#include <iostream>

using std::cout;
using std::endl;

int main() {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  int option_value;
  unsigned int option_len = sizeof(int);
  cout << "option_len before: " << option_len << endl;
  int ret = getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &option_value, &option_len);
  cout << "RCVBUF" << endl;
  cout << "ret: " << ret << endl;
  cout << "option_value: " << option_value << endl;
  cout << "option_len: " << option_len << endl << endl;
  ret = getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &option_value, &option_len);
  cout << "SNDBUF" << endl;
  cout << "ret: " << ret << endl;
  cout << "option_value: " << option_value << endl;
  cout << "option_len: " << option_len << endl;

  return 0;
}
