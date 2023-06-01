
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include "../src/RequestMessage.hpp"
#include <iostream>

int main() {
  int t;
  RequestMessage request_message(t);
  int fd = open("test", O_RDONLY);
  if (fd <= 0)
    return 0;
  char buf[8000];

  int a = 0;
  while (1) {
    a++;
    int rd = read(fd, buf, sizeof(buf));
    auto ret =  request_message.parse(buf, rd);
    std::cout << "METHOD : " << request_message.getMethod() << '\n';
    std::cout << "URI : " << request_message.getUri() << '\n';
    std::cout << "PROTOCOl : " << request_message.getAProtocol() << '\n';
    auto map(request_message.getHeaders());
    for(auto i : map) {
      std::cout << "[" << i.first << "][" << i.second << "]\n";
    }
    auto body = request_message.getBody();
    std::cout << "body : ";
    for (auto i : body) {
      std::cout << i;
    }
    std::cout << '\n';
    if (ret == SUCCESS)
      break ;
  }
}