#include <sys/event.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// kqueue
// int main(int argc, char ** argv) {
//   struct kevent event; /* Event we want to monitor */
//   struct kevent tevent; /* Event triggered */
//   int kq, fd, ret;

//   if (argc != 2)
//     err(EXIT_FAILURE, "Usage: %s path\n", argv[0]);
//   fd = open(argv[1], O_RDONLY);
//   if (fd == -1)
//     err(EXIT_FAILURE, "Failed to open '%s'", argv[1]);

//   /* Create kqueue. */
//   kq = kqueue();
//   if (kq == -1)
//     err(EXIT_FAILURE, "kqueue() failed");

//   /* Initialize kevent structure. */
//   EV_SET( & event, fd, EVFILT_VNODE, EV_ADD | EV_CLEAR, NOTE_WRITE,
//     0, NULL);
//   /* Attach event to the kqueue. */
//   ret = kevent(kq, & event, 1, NULL, 0, NULL);
//   if (ret == -1)
//     err(EXIT_FAILURE, "kevent register");
//   if (event.flags & EV_ERROR)
//     errx(EXIT_FAILURE, "Event error: %s", strerror(event.data));

//   for (;;) {
//     /* Sleep until something happens. */
//     ret = kevent(kq, NULL, 0, & tevent, 1, NULL);
//     if (ret == -1) {
//       err(EXIT_FAILURE, "kevent wait");
//     } else if (ret > 0) {
//       printf("Something was written in '%s'\n", argv[1]);
//     }
//   }
// }

// select
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

#define BUF_SIZE 100

int main(int argc, char * argv[]) {
  int serv_sock, clnt_sock; // fd : file descriptor
  struct sockaddr_in serv_addr, clnt_addr;
  struct timeval timeout;
  fd_set reads, cpy_reads;
  int fd_max, fd_num;
  socklen_t addr_size;
  int i, str_len;

  char buf[BUF_SIZE];

  if (argc != 2) {
    printf("Usage : ./program_name <port>\n");
    exit(1);
  }

  // socket
  serv_sock = socket(PF_INET, SOCK_STREAM, 0);
  if (serv_sock == -1) {
    perror("error : failed socket()");
  }
  fcntl(serv_sock, F_SETFL, O_NONBLOCK);
  memset( & serv_addr, 0, sizeof(serv_addr)); // 메모리 초기화
  serv_addr.sin_family = AF_INET; // 주소 체계 저장
  serv_addr.sin_port = htons(atoi(argv[1])); // 인자로 받은 port 번호

  // sockaddr* : sockaddr_in / sockaddr_un 이든 형변환
  if (bind(serv_sock, (struct sockaddr * ) & serv_addr, sizeof(serv_addr)) == -1) { // 소켓에 주소 할당
    perror("error : failed bind()");
    return 0;
  }

  if (listen(serv_sock, 5) == -1) { // 클라이언트 연결 대기 & 요청 queue에 저장
    perror("error : failed listen()");
    return 0;
  }

  // select I/O
  FD_ZERO( & reads); // 0으로 초기화
  FD_SET(serv_sock, & reads); // fd에 해당하는 bit 세팅
  fd_max = serv_sock;


  while (1)
  {
    // 이전 상태 저장
    cpy_reads = reads;
    timeout.tv_sec = 5;
    timeout.tv_usec = 50000;

    // select(nfds, readfds, write, err, timeout)
    if ((fd_num = select(fd_max + 1, & cpy_reads, 0, 0, & timeout)) == -1) {
      printf("fd_num : %d\n", fd_num);
      perror("select() error");
      break;
    }

    // bit 값이 1인 필드 없음 = 발견된 read data 없음
    if (fd_num == 0)
      continue;

    // 발견되면 fd 다 훑음 = O(n)
    for (i = 0; i < fd_max + 1; i++) {
      if (FD_ISSET(i, & cpy_reads)) {
        if (i == serv_sock) { // data 발생한 fd 찾으면
          printf("putin serv_sock\n");
          addr_size = sizeof(clnt_addr);

          // queue에서 연결 요청 하나씩 꺼내서 해당 client와 server socket 연결
          clnt_sock = accept(serv_sock, (struct sockaddr * ) & clnt_addr, & addr_size);
          fcntl(clnt_sock, F_SETFL, O_NONBLOCK);
          FD_SET(clnt_sock, & reads);
          if (fd_max < clnt_sock)
            // loop 돌아야 하므로 fd 큰쪽으로 맞춤
            fd_max = clnt_sock;
          printf("connected client : %d \n", clnt_sock);
        } else {
          str_len = recv(i, buf, BUF_SIZE, 0); // 데이터 수신
          if (str_len <= 0) {
            FD_CLR(i, &reads);
            close(i);
            printf("close client : %d \n", i);
          } else {
            send(i, buf, str_len, 0);
          }
        }
      }
    }
  }
  close(serv_sock);
  return 0;
}