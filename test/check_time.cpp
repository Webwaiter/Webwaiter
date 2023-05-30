#include <ctime>
#include <iostream>
#include <unistd.h>

void updateTime(time_t &cur_time) {
  cur_time = time(NULL);
}
double getTimeOut(time_t &base_time) {
  return (difftime(time(NULL), base_time));
}
int main(void) {
  time_t a;
  updateTime(a);
  std::cout << "before: " << a << std::endl;
  sleep(2);
  std::cout << getTimeOut(a) << std::endl;
  updateTime(a);
  std::cout << "before: " << a << std::endl;
  return 0;
}