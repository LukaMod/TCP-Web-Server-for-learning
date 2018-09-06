#include "web.h"

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/resource.h>

#include "error.h"
#include "event_handler.h"

void handle_request(int kq, int fd, process_t* proc) {
    ssize_t n;
    int     flag = 0;
    if ( (n = read(fd, proc->buf, BUF_SIZE)) == -1) {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            err_sys("read error");            
    } else if (n == 0) {    /* EOF */
        flag = 1; 
    } else 
        proc->last += n;
    while (proc->first != proc->last) {
        if (*proc->first++ == '\n') {
            flag = 1;
            break;
        }
    }
    
    if (flag == 1) {
        proc->last = proc->first;
        *proc->first = 0;
        enable_event(kq, fd, kWrite);
    }
}

void handle_reply(int fd, process_t* proc) {
    ssize_t n;
    int     nwritten = atol(proc->buf);
    char    result[MAX_N];

    if (nwritten <=0 || nwritten > MAX_N)
        err_quit("client request for %d bytes\n", nwritten);
    
    while (1) {
        if ( (n = Writen(fd, result, nwritten)) == nwritten) {
            proc->first = proc->last = proc->buf;
            Close(fd);
            break;
        }
        else
            nwritten -= n;
    }
}

void sig_chld(int signo) {
    pid_t   pid;
    int     stat;

    while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        /* printf("child %d terminated\n", pid); */
    }
}

#ifndef HAVE_GETRUSAGE_PROTO
int getrusage(int, struct rusage *);
#endif

void pr_cpu_time(void) {
    double          user, sys;
    struct rusage   myusage, childusage;

    if (getrusage(RUSAGE_SELF, &myusage) < 0)
        err_sys("getrusage error");
    if (getrusage(RUSAGE_CHILDREN, &childusage) < 0)
        err_sys("getrusage error");
    
    user = (double) myusage.ru_utime.tv_sec + myusage.ru_utime.tv_usec / 1000000.0;
    user += (double) childusage.ru_utime.tv_sec + childusage.ru_utime.tv_usec / 1000000.0;

    sys = (double) myusage.ru_stime.tv_sec + myusage.ru_stime.tv_usec / 1000000.0;
    sys += (double) childusage.ru_stime.tv_sec + childusage.ru_stime.tv_usec / 1000000.0;

    printf("\nuser time = %g, sys time = %g\n", user, sys);
}

void single_cpu_time(void) {
    double          user, sys;
    struct rusage   usage;

    if (getrusage(RUSAGE_SELF, &usage) < 0)
        err_sys("getrusage error");

    user = (double) usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1000000.0;

    sys = (double) usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1000000.0;

    printf("\nuser time = %g, sys time = %g\n", user, sys);

    exit(0);
}

void sig_int2(int signo) {
    void single_cpu_time(void);
    single_cpu_time();
    exit(0);
}

void sig_int(int signo) {
    void pr_cpu_time(void);
    pr_cpu_time();
    exit(0);
}

void sig_int3(int signo) {
    int     i;
    void    pr_cpu_time(void);

    pr_cpu_time();
    for (i = 0; i < nthreads; ++i)
        printf("thread %d, %ld connections\n", i, tptr[i].thread_count);
    
    exit(0);
}

void web_child(int sockfd) {
    int     ntowrite;
    ssize_t nread;
    char    line[BUF_SIZE], result[MAX_N];

    for ( ; ; ) {
        if ( (nread = Readline(sockfd, line, BUF_SIZE)) == 0)
            return;
        ntowrite = atol(line);
        if (ntowrite <= 0 || ntowrite > MAX_N)
            err_quit("client request for %d bytes", ntowrite);
        
        Writen(sockfd, result, ntowrite);
    }
}

void thread_make(int i) {
    void* thread_main(void* );

    Pthread_create(&tptr[i].thread_tid, NULL, &thread_main, (void* ) i);
}

void* thread_main(void* arg) {
    int             connfd;
    void            web_child(int);
    socklen_t       clilen;
    struct sockaddr *cliaddr;

    cliaddr = Malloc(sizeof(addrlen));

    printf("thread %d starting\n", (int) arg);
    for ( ; ; ) {
        clilen = addrlen;
        
        Pthread_mutex_lock(&mlock);
        connfd = Accept(listenfd, cliaddr, &clilen);
        Pthread_mutex_unlock(&mlock);

        tptr[(int) arg].thread_count++;

        web_child(connfd);  /* process request */
        Close(connfd);
    }
}
