#ifndef _UNP_H_
#define _UNP_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define MAXLINE 2048
#define LISTENQ 5

/* iofunc.c */
ssize_t readn(int fd, void* vptr, size_t n);
ssize_t writen(int fd, const void* vptr, size_t n);
ssize_t readline(int fd, void* vptr, size_t maxlen);
ssize_t readlinebuf(void** vptrptr);

#endif
