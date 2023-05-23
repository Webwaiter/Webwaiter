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
    std::cout << "done" << '\n';
    std::cout << test.getServerProgramName() << '\n';
    std::cout << test.getHttpVersion() << '\n';
  }
  catch (int) {
    std::cout << "fail to construct class" << '\n';
  }

  
  return 0;
}