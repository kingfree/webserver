#include "log.h"

#include <stdarg.h>
#include <syslog.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define MAXLINE 4096

void log_info(const char *fmt, ...)
{
    va_list p;
    static char buff[MAXLINE];

    va_start(p, fmt);
    vsnprintf(buff, sizeof(buff), fmt, p);
    fprintf(stderr, "信息: %s\n", buff);
    va_end(p);

    va_start(p, fmt);
    syslog(LOG_INFO, fmt, p);
    va_end(p);
}

void log_error(const char *fmt, ...)
{
    va_list p;
    static char buff[MAXLINE];
    size_t n;
    int errnod;

    errnod = errno;

    va_start(p, fmt);
    vsnprintf(buff, sizeof(buff), fmt, p);
    n = strlen(buff);
    snprintf(buff + n, sizeof(buff) - n, ": %s", strerror(errnod));
    fprintf(stderr, "错误: %s\n", buff);
    va_end(p);

    va_start(p, fmt);
    syslog(LOG_ERR, fmt, p);
    va_end(p);
}
