#include "sock_unix.h"

#include <netdb.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#include "error.h"

/* for my_read() */

static int  read_cnt;
static char *read_ptr;
static char read_buf[BUF_SIZE];

void init_proc_arr(process_t* arr, size_t n) {
    int i;
    for (i = 0; i != n; ++i) {
        arr[i].buf = (char* )malloc(BUF_SIZE * sizeof(char));
        if (arr[i].buf == NULL)
            err_sys("malloc error");
        arr[i].first = arr[i].last = arr[i].buf;
    }
}

void free_proc_arr(process_t* arr, size_t n) {
    int i;
    for (i = 0; i != n; ++i) {
        free(arr[i].buf);
        arr[i].buf = arr[i].first = arr[i].last = NULL;
    }
}

int tcp_listen(const char* hostname, const char* service, socklen_t* addrlen_p) {
    int             listenfd, n;    /* n for gai_strerror */
    const int       on = 1;         /* on for setsockopt() */
    struct addrinfo hints, *res, *res_save;

    /* initialization */
    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ( (n = getaddrinfo(hostname, service, &hints, &res)) != 0)    
        err_quit("tcp_listen error for %s, %s: %s", hostname, service, gai_strerror(n));
    
    res_save = res;

    do {
        if ( (listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
            continue;   /* error, try next one */
        
        Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

        if (bind(listenfd, res->ai_addr, res->ai_addrlen) == 0)
            break;      /* success */
            
        /* bind error, close and try next one */    
        Close(listenfd);

    } while ( (res = res->ai_next) != NULL);

    if (res == NULL)    /* error from final socket() or bind() */
        err_quit("tcp_listen error for %s, %s", hostname, service);
    
    Listen(listenfd, 2048);

    if (addrlen_p)      /* return size of protocol address */
        *addrlen_p = res->ai_addrlen;

    freeaddrinfo(res_save);
    return listenfd;
}

void setnonblock(int fd) {
    int flags;

    if ( (flags = fcntl(fd, F_GETFL, 0)) == -1)
        err_sys("F_GETFL error");
        

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        err_sys("F_SETFL error");
}

int tcp_connect(const char* hostname, const char* service) {
    int             sockfd, n;
    struct addrinfo hints, *res, *res_save;

    /* initialize */
    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ( (n = getaddrinfo(hostname, service, &hints, &res)) != 0)
        err_quit("tcp_connect error for %s, %s: %s", hostname, service, gai_strerror(n));
    
    res_save = res;

    do {
        if ( (sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
            continue;
        
        if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
            break;

        Close(sockfd);
    } while ( (res = res->ai_next) != NULL);

    if (res == NULL)
        err_sys("tcp_connect error for %s, %s", hostname, service);
    
    freeaddrinfo(res_save);
    return sockfd;
}

int Socket(int family, int type, int protocol) {
    int sockfd;
    if ( (sockfd = socket(family, type, protocol)) == -1)
        err_sys("socket error");
    return sockfd;
}

void Connect(int sockfd, const struct sockaddr* address, socklen_t address_len) {
    if (connect(sockfd, address, address_len) == -1)
        err_sys("connect error");
}

void Bind(int sockfd, const struct sockaddr* address, socklen_t address_len) {
    if (bind(sockfd, address, address_len) == -1)
        err_sys("bind error");
}

void Listen(int listenfd, int backup) {
    if (listen(listenfd, backup) == -1)
        err_sys("listen error");
}

int Accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen_p) {
    int fd;
again:
    if ( (fd = accept(sockfd, addr, addrlen_p)) == -1) {
#ifdef EPROTO
        if (errno == EPROTO || errno == ECONNABORTED)
#else
        if (errno == ECONNABORTED)
#endif
            goto again;
        else
            err_sys("accept error");
    }

    return fd;
}

void Shutdown(int sockfd, int how) {
    if (shutdown(sockfd, how) == -1)
        err_sys("shutdown error");
}

void Setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen) {
    if (setsockopt(sockfd, level, optname, optval, optlen) == -1)
        err_sys("setsockopt error");
}

void Close(int fd) {
    if (close(fd) == -1)
        err_sys("close error");
}

int Fcntl(int fd, int cmd, int arg) {
    int n;
    if ( (n = fcntl(fd, cmd, arg)) == -1)
        err_sys("fcntl error");
    return n;
}

pid_t Fork(void) {
    pid_t pid;
    if ( (pid = fork()) == -1)
        err_sys("fork error");
    return pid;
}

ssize_t Read(int fd, void* buf, size_t nbytes) {
    ssize_t n;
    if ( (n = read(fd, buf, nbytes)) == -1)
        err_sys("read error");
    return n;
}

ssize_t Write(int fd, const void* buf, size_t nbytes) {
    ssize_t n;
    if ( (n = write(fd, buf, nbytes)) == -1)
        err_sys("write error");
    return n;
}

ssize_t readn(int fd, void* buf, size_t nbytes) {
    ssize_t nread;
    size_t  nleft;
    char*   p;

    nleft = nbytes;
    p = buf;

    while (nleft > 0) {
        if ( (nread = read(fd, p, nleft)) == -1) {
            if (errno == EINTR)
                nread = 0;
            else
                return -1;
        } else if (nread == 0)
            break;
        
        nleft -= nread;
        p += nread;
    }
    return nbytes - nleft;
}

ssize_t Readn(int fd, void* buf, size_t nbytes) {
    ssize_t n;
    if ( (n = readn(fd, buf, nbytes)) == -1)
        err_sys("readn error");
    return n;
}

ssize_t writen(int fd, const void* buf, size_t nbytes) {
    size_t      nleft;
    ssize_t     nwritten;
    const char* ptr;

    nleft = nbytes;
    ptr = buf;
    
    while (nleft > 0) {
        if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0;   /* continue */
            else
                return -1;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return nbytes;
}

ssize_t Writen(int fd, const void* buf, size_t nbytes) {
    ssize_t n;
    if ( (n = writen(fd, buf, nbytes)) == -1)
        err_sys("writen error");
    return n;
}

static ssize_t my_read(int fd, char* ptr) {
    if (read_cnt <= 0) {
        again:
            if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
                if (errno == EINTR)
                    goto again;
                return -1;
            } else if (read_cnt == 0)
                return 0;
            read_ptr = read_buf;
    }
    --read_cnt;
    *ptr = *read_ptr++;
    return 1;
}

ssize_t readline(int fd, void* buf, size_t nbytes) {
    ssize_t n, rc;
    char    c, *ptr;

    ptr = buf;
    for (n = 1; n < nbytes; ++n) {
        if ( (rc = my_read(fd, &c)) == 1) {
            *ptr++ = c;
            if (c == '\n')
                break;
        } else if (rc == 0) {
            *ptr = 0;       /* EOF */
            return n - 1;   
        } else
            return -1;
    }
    *ptr = 0;
    return n;
}

ssize_t Readline(int fd, void* buf, size_t nbytes) {
    ssize_t n;
    if ( (n = readline(fd, buf, nbytes)) == -1)
        err_sys("readline error");
    return n;
}

void* Malloc(size_t n) {
    void* p;
    if ( (p = malloc(n)) == NULL)
        err_sys("malloc error");
    return p;
}

void* Calloc(size_t count, size_t size) {
    void* p;
    if ( (p = calloc(count, size)) == NULL)
        err_sys("calloc error");
    return p;
}

Sigfunc* signal(int signo, Sigfunc* func) {
    struct sigaction act, oact;

    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    if (signo == SIGALRM) {

#ifdef SA_INTERRUPT
        act.sa_flags |= SA_INTERRUPT;
#endif

    } else {

#ifdef SA_RESTART
        act.sa_flags |= SA_RESTART;
#endif

    }
    if (sigaction(signo, &act, &oact) < 0)
        return SIG_ERR;
    return (oact.sa_handler);
}

Sigfunc* Signal(int signo, Sigfunc* func) {
    Sigfunc* sfun;
    if ( (sfun = signal(signo, func)) == SIG_ERR)
        err_sys("signal error");
    return sfun;
}

void Pthread_create(pthread_t* tid, const pthread_attr_t* attr, void* (*func)(void* ), void* arg) {
    int n;

    if ( (n = pthread_create(tid, attr, func, arg)) == 0)
        return;
    
    errno = n;
    err_sys("pthread_create error");
}

void Pthread_mutex_lock(pthread_mutex_t* mutex) {
    int n;

    if ( (n = pthread_mutex_lock(mutex)) == 0)
        return;
    
    errno = n;
    err_sys("pthread_mutex_lock error");
}

void Pthread_mutex_unlock(pthread_mutex_t* mutex) {
    int n;

    if ( (n = pthread_mutex_unlock(mutex)) == 0)
        return;
    
    errno = n;
    err_sys("pthread_mutex_unlcok error");
}
