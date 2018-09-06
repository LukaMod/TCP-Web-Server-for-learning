#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "sock_unix.h"
#include "error.h"

int main(int argc, char** argv) {
    int     i, j, nchildren, nloops, nbytes;
    int     fd;
    ssize_t n;
    pid_t   pid;
    char    request[BUF_SIZE], reply[MAX_N];

    if (argc != 6)
        err_quit("usage: client <hostname or IPaddress> <port> <#children> "
                "<#loops/child> <#bytes/request>");

    nchildren = atoi(argv[3]);
    nloops = atoi(argv[4]);
    nbytes = atoi(argv[5]);

    snprintf(request, sizeof(request), "%d\n", nbytes);

    for (i = 0; i < nchildren; ++i) {
        if ( (pid = Fork()) == 0) { /* child */
            for (j = 0; j < nloops; ++j) {
                fd = tcp_connect(argv[1], argv[2]);

                Write(fd, request, strlen(request));

                if (Readn(fd, reply, nbytes) != nbytes)
                    err_quit("server returned %d bytes", n);

                Close(fd); 
            }
            printf("child %d done\n", i);
            exit(0);
        }
        /* parent loops around to fork() again */
    }

    while (wait(NULL) > 0)
        ;
    if (errno != ECHILD)
        err_sys("wait error");
    exit(0);
}