#include "sock_unix.h"

#include <pthread.h>

void handle_request(int kq, int fd, process_t* proc);

void handle_reply(int fd, process_t* proc);

void sig_chld(int signo);

void sig_int(int signo);

/* SIGINT handler for single process */

void sig_int2(int signo);

/* for thread version */

void sig_int3(int signo);

void web_child(int sockfd);

/* pthread */

typedef struct {
    pthread_t   thread_tid;     /* thread ID */
    long        thread_count;   /* # connections handled */
} thread;

thread* tptr;

int             listenfd, nthreads;
socklen_t       addrlen;
pthread_mutex_t mlock;

void thread_make(int i);

void* thread_main(void *arg);
