#ifndef SOCK_H
#define SOCK_H

#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE    2048
#define MAX_PROC    2048
#define MAX_N       16384

typedef struct process {
    char    *buf, *first, *last;
} process_t;

/* initialize process array */

void init_proc_arr(process_t* arr, size_t n);

/* free buffer and reset fd */

void free_proc_arr(process_t* arr, size_t n);

/*
 * @hostname is either a valid host name or an IPv4/Ipv6 adress
 * @service is either a port number or a service
 * @addrlen_p if non-null, returns the size of address
 * 
 * at least, one of hostname and service must be non-null
 * 
 * @return tcp listenfd
 */

int tcp_listen(const char* hostname, const char* service, socklen_t* addrlen_p);

int tcp_connect(const char* hostname, const char* service);

void setnonblock(int fd);

/* socket wrapper */

int Socket(int family, int type, int protocol);

void Connect(int sockfd, const struct sockaddr* address, socklen_t address_len);

void Bind(int sockfd, const struct sockaddr* address, socklen_t address_len);

void Listen(int listenfd, int backup);

int Accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen_p);

void Shutdown(int fd, int how);

void Setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen);

/* unistd wrapper */

void Close(int fd);

int Fcntl(int fd, int cmd, int arg);

pid_t Fork(void);

ssize_t Read(int fd, void* buf, size_t nbytes);

ssize_t Write(int fd, const void* buf, size_t nbytes);

/* read nbytes from a fd */

ssize_t readn(int fd, void* buf, size_t nbytes);

ssize_t Readn(int fd, void* buf, size_t nbytes);

/* write nbytes to a fd */

ssize_t writen(int fd, const void* buf, size_t nbytes);

ssize_t Writen(int fd, const void* buf, size_t nbytes);

ssize_t readline(int fd, void* buf, size_t maxlen);

ssize_t Readline(int fd, void* buf, size_t nbytes);

/* malloc */

void* Malloc(size_t n);

void* Calloc(size_t count, size_t size);

/* signal */

typedef void Sigfunc(int);

Sigfunc* signal(int signo, Sigfunc* func);

Sigfunc* Signal(int signo, Sigfunc* func);

/* pthread */

void Pthread_create(pthread_t* tid, const pthread_attr_t* attr, void* (*func)(void* ), void* arg);

void Pthread_mutex_lock(pthread_mutex_t* mutex);

void Pthread_mutex_unlock(pthread_mutex_t* mutex);

#endif
