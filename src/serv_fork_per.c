#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include "sock_unix.h"
#include "error.h"
#include "web.h"

int main(int argc, char** argv) {
    int             listenfd, connfd;
    pid_t           childpid;
    struct sockaddr *cliaddr;
    socklen_t       clilen, addrlen;

    if (argc == 2)
        listenfd = tcp_listen(NULL, argv[1], &addrlen);
    else if (argc == 3)
        listenfd = tcp_listen(argv[1], argv[2], &addrlen);
    else
        err_quit("usage: serv_fork_per [ <host> ] <port#>");
    
    cliaddr = Malloc(addrlen);

    Signal(SIGCHLD, sig_chld);
    Signal(SIGINT, sig_int);

    for ( ; ; ) {
        clilen = addrlen;
        if ( (connfd = accept(listenfd, cliaddr, &clilen)) == -1) {
            if (errno == EINTR)
                continue;
            else
                err_sys("accept error");
        }
        if ( (childpid = Fork()) == 0) {         
            Close(listenfd);
            web_child(connfd);
            exit(0);
        }
        Close(connfd);
    }
}