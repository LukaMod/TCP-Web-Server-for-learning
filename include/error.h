#ifndef ERROR_H
#define ERROR_H

/* print msg containing errno and terminate */

void err_sys(const char* fmt, ...);

/* print msg and terminate */

void err_quit(const char* fmt, ...);

/* print msg */

void err_msg(const char* fmt, ...);

#endif
