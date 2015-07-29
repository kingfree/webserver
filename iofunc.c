#include "unp.h"

ssize_t readn(int fd, void* buf, size_t n)
{
    size_t nleft;
    ssize_t nread;
    char* p;

    p = buf;
    nleft = n;
    while (nleft > 0) {
        if ((nread = read(fd, p, nleft)) < 0) {
            if (errno == EINTR)
                nread = 0;
            else
                return -1;
        } else if (nread == 0)
            break;

        nleft -= nread;
        p += nread;
    }
    return n - nleft;
}

ssize_t writen(int fd, const void* buf, size_t n)
{
    size_t nleft;
    ssize_t nwritten;
    const char* p;

    p = buf;
    nleft = n;
    while (nleft > 0) {
        if ((nwritten = write(fd, p, nleft)) <= 0) {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0;
            else
                return -1;
        }

        nleft -= nwritten;
        p += nwritten;
    }
    return n;
}

static ssize_t read_cnt;
static char* read_p;
static char read_buf[MAXLINE];

static ssize_t my_read(int fd, char* p)
{
    if (read_cnt <= 0) {
    again:
        if ((read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
            if (errno == EINTR) goto again;
            return -1;
        } else if (read_cnt == 0)
            return 0;
        read_p = read_buf;
    }

    read_cnt--;
    *p = *read_p++;
    return 1;
}

ssize_t readline(int fd, void* buf, size_t maxlen)
{
    size_t n;
    ssize_t nread;
    char c, *p;

    p = buf;
    for (n = 1; n < maxlen; n++) {
        if ((nread = my_read(fd, &c)) == 1) {
            *p++ = c;
            if (c == '\n') break;
        } else if (nread == 0) {
            *p = 0;
            return n - 1;
        } else
            return -1;
    }

    *p = 0;
    return n;
}

ssize_t readlinebuf(void** bufp)
{
    if (read_cnt) *bufp = read_p;
    return read_cnt;
}
