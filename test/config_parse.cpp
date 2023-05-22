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
  Config test(argv[1]);
  
  return 0;
}