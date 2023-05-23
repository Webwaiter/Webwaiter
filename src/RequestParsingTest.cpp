
#include <fcntl.h>
#include <unistd.h>
#include "../src/RequestMessage.hpp"
#include <iostream>

int main() {
  RequestMessage request_message;
  int fd = open("test", O_RDONLY);
  if (fd <= 0)
    return 0;
  char buf[8093];
  while (1) {
    int rd = read(fd, buf, sizeof(buf) - 1);
    if (rd == 0)
      break ;
    buf[rd] = '\0';
    request_message.parse(buf);
    std::cout << "METHOD : " << request_message.getMethod1() << '\n';
    std::cout << "URI : " << request_message.getUri() << '\n';
    std::cout << "PROTOCOl : " << request_message.getAProtocol() << '\n';
  }
}