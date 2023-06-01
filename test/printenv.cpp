#include <iostream>
#include <string>

using std::cout;
using std::cerr;
using std::endl;

extern char** environ;

int main() {
  cerr << "child output:" << endl;
  for (char** p = environ; *p; ++p)
    cerr << *p << endl;
  cerr << endl;
  std::string line;
  while (getline(std::cin, line))
    cout << line << endl;
  return 1;
}
