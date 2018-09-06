#include "error.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define MAXLINE 1024

#if __STDC__VERSION__ > 199409L
#define HAS_VSNPRINTF
#endif

static void err_do(int errno_flag, const char* fmt, va_list ap);

void err_sys(const char* fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    err_do(1, fmt, ap);
    va_end(ap);
    exit(1);
}

void err_quit(const char* fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    err_do(0, fmt, ap);
    va_end(ap);
    exit(1);
}

void err_msg(const char* fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    err_do(0, fmt, ap);
    va_end(ap);
}

void err_do(int errno_flag, const char* fmt, va_list ap) {
    char    buf[MAXLINE + 1];
    int     errno_save = errno;

#ifdef HAS_VSNPRINTF
    vsnprintf(buf, MAXLINE, fmt, ap);   /* safe */
#else
    vsprintf(buf, fmt, ap);             /* not safe */
#endif

    if (errno_flag) {
        int n = strlen(buf);

#ifdef HAS_VSNPRINTF
        snprintf(buf + n, MAXLINE - n, ": %s", strerror(errno_save));   /* safe */
#else
        sprintf(buf + n, ": %s", strerror(errno_save)); /* not safe */
#endif

    }
    
    strcat(buf, "\n");

    fflush(stdout);
    fputs(buf, stderr);
    fflush(stderr);
}
