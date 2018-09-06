#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <sys/event.h>

#define MAX_EVENTS   256

/* event filter, support bit operation */

extern const int kRead;
extern const int kWrite;

int Kqueue(void);

int Kevent(int kq, const struct kevent* changelist, int nchanges, struct kevent* eventlist, int nevents, const struct timespec* timeout);

void register_event(int kqueuefd, int fd, int filter);

void remove_event(int kqueuefd, int fd, int filter);

void enable_event(int kqueuefd, int fd, int filter);

void disable_event(int kqueuefd, int fd, int filter);

#endif
