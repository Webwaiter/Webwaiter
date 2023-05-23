#include <iostream>
#include <string>

using std::cout;
using std::endl;

extern char** environ;

int main() {
  for (char** p = environ; *p; ++p)
    cout << *p << endl;
  cout << endl;
  std::string line;
  while (getline(std::cin, line))
    cout << line << endl;
  return 0;
}
