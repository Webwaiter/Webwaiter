// #include <sys/socket.h>
// #include <sys/un.h>
// #include <sys/event.h>
// #include <netdb.h>
// #include <assert.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <stdio.h>
// #include <errno.h>

// #include <sys/types.h>
// #include <sys/stat.h>

// int main(int argc, const char * argv[]) {
    
//     // Macos automatically binds both ipv4 and 6 when you do this.
//     struct sockaddr_in6 addr = {};
//     addr.sin6_len = sizeof(addr);
//     addr.sin6_family = AF_INET6;
//     addr.sin6_addr = in6addr_any; //(struct in6_addr){}; // 0.0.0.0 / ::
//     addr.sin6_port = htons(9999);
    
//     int localFd = socket(addr.sin6_family, SOCK_STREAM, 0);
//     assert(localFd != -1);
    
//     int on = 1;
//     setsockopt(localFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
//     if (bind(localFd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
//         perror("bind");
//         return 1;
//     }
//     assert(listen(localFd, 5) != -1);

//     int kq = kqueue();
    
//     struct kevent evSet;
//     EV_SET(&evSet, localFd, EVFILT_READ, EV_ADD, 0, 0, NULL);
//     assert(-1 != kevent(kq, &evSet, 1, NULL, 0, NULL));
    
//     int junk = open("some.big.file", O_RDONLY);
    
//     uint64_t bytes_written = 0;

//     struct kevent evList[32];
//     while (1) {
//         // returns number of events
//         int nev = kevent(kq, NULL, 0, evList, 32, NULL);
// //        printf("kqueue got %d events\n", nev);
        
//         for (int i = 0; i < nev; i++) {
//             int fd = (int)evList[i].ident;
            
//             if (evList[i].flags & EV_EOF) {
//                 printf("Disconnect\n");
//                 close(fd);
//                 // Socket is automatically removed from the kq by the kernel.
//             } else if (fd == localFd) {
//                 struct sockaddr_storage addr;
//                 socklen_t socklen = sizeof(addr);
//                 int connfd = accept(fd, (struct sockaddr *)&addr, &socklen);
//                 assert(connfd != -1);
                
//                 // Listen on the new socket
//                 EV_SET(&evSet, connfd, EVFILT_READ, EV_ADD, 0, 0, NULL);
//                 kevent(kq, &evSet, 1, NULL, 0, NULL);
//                 printf("Got connection!\n");
                
//                 int flags = fcntl(connfd, F_GETFL, 0);
//                 assert(flags >= 0);
//                 fcntl(connfd, F_SETFL, flags | O_NONBLOCK);

//                 // schedule to send the file when we can write (first chunk should happen immediately)
//                 EV_SET(&evSet, connfd, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, NULL);
//                 kevent(kq, &evSet, 1, NULL, 0, NULL);

//             } else if (evList[i].filter == EVFILT_READ) {
//                 // Read from socket.
//                 char buf[1024];
//                 size_t bytes_read = recv(fd, buf, sizeof(buf), 0);
//                 printf("read %zu bytes\n", bytes_read);
                
                
//             } else if (evList[i].filter == EVFILT_WRITE) {
// //                printf("Ok to write more!\n");
                
//                 off_t offset = (off_t)evList[i].udata;
//                 off_t len = 0;//evList[i].data;
//                 if (sendfile(junk, fd, offset, &len, NULL, 0) != 0) {
// //                    perror("sendfile");
// //                    printf("err %d\n", errno);
                    
//                     if (errno == EAGAIN) {
//                         // schedule to send the rest of the file
//                         EV_SET(&evSet, fd, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, (void *)(offset + len));
//                         kevent(kq, &evSet, 1, NULL, 0, NULL);
//                     }
//                 }
//                 bytes_written += len;
//                 printf("wrote %lld bytes, %lld total\n", len, bytes_written);
//             }
//         }
//     }
    
//     return 0;
// }

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <map>
#include <vector>

using namespace std;

void exit_with_perror(const string& msg)
{
    cerr << msg << endl;
    exit(EXIT_FAILURE);
}

void change_events(vector<struct kevent>& change_list, uintptr_t ident, int16_t filter,
        uint16_t flags, uint32_t fflags, intptr_t data, void *udata)
{
    struct kevent temp_event;

    EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
    change_list.push_back(temp_event);
}

void disconnect_client(int client_fd, map<int, string>& clients)
{
    cout << "client disconnected: " << client_fd << endl;
    close(client_fd);
    clients.erase(client_fd);
}

int main()
{
    /* init server socket and listen */
    int server_socket;
    struct sockaddr_in server_addr;

    if ((server_socket = socket(PF_INET, SOCK_STREAM, 0)) == -1)
        exit_with_perror("socket() error\n" + string(strerror(errno)));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(8080);
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
        exit_with_perror("bind() error\n" + string(strerror(errno)));

    if (listen(server_socket, 5) == -1)
        exit_with_perror("listen() error\n" + string(strerror(errno)));
    fcntl(server_socket, F_SETFL, O_NONBLOCK);
    
    /* init kqueue */
    int kq;
    if ((kq = kqueue()) == -1)
        exit_with_perror("kqueue() error\n" + string(strerror(errno)));


    map<int, string> clients; // map for client socket:data
    vector<struct kevent> change_list; // kevent vector for changelist
    struct kevent event_list[8]; // kevent array for eventlist

    /* add event for server socket */
    change_events(change_list, server_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    cout << "echo server started" << endl;

    /* main loop */
    int new_events;
    struct kevent* curr_event;
    while (1)
    {
        /*  apply changes and return new events(pending events) */
        new_events = kevent(kq, &change_list[0], change_list.size(), event_list, 8, NULL);
        if (new_events == -1)
            exit_with_perror("kevent() error\n" + string(strerror(errno)));

        change_list.clear(); // clear change_list for new changes

        for (int i = 0; i < new_events; ++i)
        {
            curr_event = &event_list[i];

            /* check error event return */
            if (curr_event->flags & EV_ERROR)
            {
                if (curr_event->ident == server_socket)
                    exit_with_perror("server socket error");
                else
                {
                    cerr << "client socket error" << endl;
                    disconnect_client(curr_event->ident, clients);
                }
            }
            else if (curr_event->filter == EVFILT_READ)
            {
                if (curr_event->ident == server_socket)
                {
                    /* accept new client */
                    int client_socket;
                    if ((client_socket = accept(server_socket, NULL, NULL)) == -1)
                        exit_with_perror("accept() error\n" + string(strerror(errno)));
                    cout << "accept new client: " << client_socket << endl;
                    fcntl(client_socket, F_SETFL, O_NONBLOCK);

                    /* add event for client socket - add read && write event */
                    change_events(change_list, client_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
                    change_events(change_list, client_socket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
                    clients[client_socket] = "";
                }
                else if (clients.find(curr_event->ident)!= clients.end())
                {
                    /* read data from client */
                    char buf[1024];
                    int n = read(curr_event->ident, buf, sizeof(buf));

                    if (n <= 0)
                    {
                        if (n < 0)
                            cerr << "client read error!" << endl;
                        cerr << "read is zero" << endl;
                        disconnect_client(curr_event->ident, clients);
                    }
                    else
                    {
                        buf[n] = '\0';
                        clients[curr_event->ident] += buf;
                        cout << "received data from " << curr_event->ident << ": " << clients[curr_event->ident] << endl;
                    }
                }
            }
            else if (curr_event->filter == EVFILT_WRITE)
            {
                /* send data to client */
                map<int, string>::iterator it = clients.find(curr_event->ident);
                if (it != clients.end())
                {
                    if (clients[curr_event->ident] != "")
                    {
                        int n;
                        if ((n = write(curr_event->ident, clients[curr_event->ident].c_str(),
                                        clients[curr_event->ident].size()) == -1))
                        {
                            cerr << "client write error!" << endl;
                            disconnect_client(curr_event->ident, clients);  
                        }
                        else
                            clients[curr_event->ident].clear();
                    }
                }
            }
        }

    }
    return (0);
}
