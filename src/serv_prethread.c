#include <stdlib.h>
#include <unistd.h>

#include "error.h"
#include "web.h"

pthread_mutex_t mlock = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char** argv) {
    int i;

    if (argc == 3)
        listenfd = tcp_listen(NULL, argv[1], &addrlen);
    else if (argc == 4)
        listenfd = tcp_listen(argv[1], argv[2], &addrlen);
    else
        err_quit("usage: ser_thread [ <host> ] <port#> <#threads>");
    
    nthreads = atoi(argv[argc - 1]);
    tptr = Calloc(nthreads, sizeof(thread));

    for (i = 0; i < nthreads; ++i)
        thread_make(i);
    
    Signal(SIGINT, sig_int3);

    for ( ; ; )
        pause();
}