#include <stdlib.h>
#include <stdio.h>
#include <sys/event.h>
#include <errno.h>
#include <unistd.h>

#include "sock_unix.h"
#include "error.h"
#include "event_handler.h"
#include "web.h"

int main(int argc, char** argv) {
    /* descriptor */
    int             kqueuefd, listenfd, connfd;

    int             i, nevents;
    struct kevent   eventlist[MAX_EVENTS], event;
    process_t       processes[MAX_PROC], *proc;
    int             offset = -2;    /* for stderr, listenfd, kqueuefd */
    ssize_t         n, nwritten;    /* for read and write */    

    // struct timespec timeout;

    // timeout.tv_sec = 5;
    // timeout.tv_nsec = 0;

    /* close unused stdin, stdout */
    if (close(STDIN_FILENO) == -1)
        err_sys("close error");

    if (argc == 2) {
        /* if didn't specify ipaddress, tcp_listen() would open two fd,
         * one for IPv6 as default and another for IPv4(I guess...),
         * so offset should be -4
         */
        offset = -3;
        listenfd = tcp_listen(NULL, argv[1], NULL);
    } else if (argc == 3)
        listenfd = tcp_listen(argv[1], argv[2], NULL);
    else
        err_quit("Usage: server <service or port>");

    kqueuefd = Kqueue();

    init_proc_arr(processes, MAX_PROC);

    setnonblock(listenfd);

    register_event(kqueuefd, listenfd, kRead);

    Signal(SIGINT, sig_int2);

    for ( ; ; ) {
        nevents = Kevent(kqueuefd, NULL, 0, eventlist, MAX_EVENTS, NULL);

        for (i = 0; i != nevents; ++i) {
            event = eventlist[i];
            if (event.ident == listenfd) {
                if ( (connfd = accept(listenfd, NULL, NULL)) == -1) {
                    if (errno != EAGAIN && errno != EWOULDBLOCK)    /* if non-block, ignore these errno */
                        err_sys("accept error");
                } else {
                    if (connfd + offset >= MAX_PROC) {
                        Close(connfd);    /* if exceeds MAX_PROC, close connfd immediately */
                    } else {            
                        setnonblock(connfd);
                        register_event(kqueuefd, connfd, kRead | kWrite);
                        disable_event(kqueuefd, connfd, kWrite);
                    }
                }
            } else {
                connfd = event.ident;
                proc = processes + connfd + offset;

                if (event.filter == EVFILT_READ) {
                    handle_request(kqueuefd, connfd, proc);
                }
                else {  /* EVFILT_WRITE */
                    handle_reply(connfd, proc);
                }
            }
        }
    }

    return 0;
}
