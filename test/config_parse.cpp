// Copyright 2023 ean, hanbkim, jiyunpar

#include <iostream>

#include "../src/Config.hpp" 
#include "../src/ServerBlock.hpp" 
#include "../src/LocationBlock.hpp"

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cout << "Usage : ./conf_test [file]\n";
    return 1;
  }
  try {
    Config test(argv[1]);
    std::cout << "----------config------------" << '\n';
    std::cout << test.getServerProgramName() << '\n';
    std::cout << test.getHttpVersion() << '\n';
    std::cout << "----------server------------" << '\n';
    std::vector<ServerBlock*> servers = test.getServerBlocks();
    std::cout << servers[0]->getDefaultErrorPages()["400"] << '\n';
    std::cout << servers[0]->getDefaultErrorPages()["500"] << '\n';
    std::cout << servers[0]->getClientBodySize() << '\n';
    std::cout << servers[0]->getServerIP() << '\n';
    std::cout << servers[0]->getServerPort() << '\n';
    std::cout << servers[0]->getServerName() << '\n';
  }
  catch (int) {
    std::cout << "fail to construct class" << '\n';
  } 
  return 0;
}