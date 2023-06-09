# Webwaiter
A simple web server that
- is written in C++
- utilizes kqueue event loop
- is single-threaded
- supports GET, POST and DELETE requests
- supports keep-alive connections
- supports HTTP/1.1
- supports IPv4
- generates directory listings
- supports redirection
### How to install and run
```
git clone https://github.com/Webwaiter/Webwaiter.git
cd Webwaiter
make
./webserv [config]
```
### Coding convention
[Google C++ Style](https://google.github.io/styleguide/cppguide.html)
