#include "event_handler.h"

#include <sys/event.h>
#include <stdlib.h>

#include "error.h"

const int kRead = 1;
const int kWrite = 2;

int Kqueue(void) {
    int kq;
    if ( (kq = kqueue()) == -1)
        err_sys("kqueue error");
    return kq;
}

int Kevent(int kq, const struct kevent* changelist, int nchanges, struct kevent* eventlist, int nevents, const struct timespec* timeout) {
    int ret;
    if ( (ret = kevent(kq, changelist, nchanges, eventlist, nevents, timeout)) == -1)
        err_sys("kevent error");
    return ret;
}

static void event_handle(int kqueuefd, int fd, int filter, int flag);

void register_event(int kqueuefd, int fd, int filter) {
    event_handle(kqueuefd, fd, filter, EV_ADD);
}

void remove_event(int kqueuefd, int fd, int filter) {
    event_handle(kqueuefd, fd, filter, EV_DELETE);
}

void enable_event(int kqueuefd, int fd, int filter) {
    event_handle(kqueuefd, fd, filter, EV_ENABLE);
}

void disable_event(int kqueuefd, int fd, int filter) {
    event_handle(kqueuefd, fd, filter, EV_DISABLE);
}

void event_handle(int kqueuefd, int fd, int filter, int flag) {
    struct kevent changelist[2];
    int n = 0;
    
    if (filter & kRead)
        EV_SET(&changelist[n++], fd, EVFILT_READ, flag, 0, 0, NULL);
    
    if (filter & kWrite)
        EV_SET(&changelist[n++], fd, EVFILT_WRITE, flag, 0, 0, NULL);

    if (kevent(kqueuefd, changelist, n, NULL, 0, NULL) == -1)
        err_sys("kevent error");        
}
