#include "unp.h"
#include <stdarg.h>
#include <syslog.h>

#define DEFAULT_PORT 80

#define koko fprintf(stderr, "ライン %d: %s() ここまで\n", __LINE__, __func__)

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

void hello(int fd)
{
    char buff[MAXLINE];
    char buf[MAXLINE];
    time_t now;
    struct tm * tm;
    int n;

    now = time(NULL);
    tm = localtime(&now);
    strftime(buf, sizeof(buf), "%G-%m-%d %T", tm);

    snprintf(buff, sizeof(buff), 
            "HTTP/1.1 200 OK\r\n"
            "Server: Time\r\n"
            "Date: %s\r\n"
            "Content-Type: text/plain;charset=UTF-8\r\n"
            "Content-Length: %ld\r\n"
            "\r\n%s\r\n\r\n",
            buf, strlen(buf), buf);
    puts(buff);

    n = send(fd, buff, strlen(buff), 0);
    if (n < 0) {
        log_error("发送数据失败 %d", n);
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    int port;
    int listen_fd, conn_fd;
    struct sockaddr_in server, client;
    int res;
    socklen_t socklen;
    int pid;

    if (argc > 1 && (port = atoi(argv[1])) < 1) port = DEFAULT_PORT;

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        log_error("建立套接字失败");
        exit(1);
    }

    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    res = bind(listen_fd, (struct sockaddr *)&server, sizeof(server));
    if (res < 0) {
        log_error("绑定描述符失败");
        exit(1);
    }

    res = listen(listen_fd, LISTENQ);
    if (res < 0) {
        log_error("监听失败");
        exit(1);
    }
    koko;

    for (;;) {
        socklen = sizeof(client);
        koko;
        conn_fd = accept(listen_fd, (struct sockaddr *)&client, &socklen);
        koko;
        if (conn_fd < 0) {
            log_error("接收数据失败");
            exit(1);
        }
        koko;
        log_info("连接来自 %s:%d", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

        pid = fork();
        if (pid == 0) {
            close(listen_fd);
            hello(conn_fd);
            exit(0);
        } else if (pid < 0) {
            log_error("创建新进程失败");
            exit(1);
        }
        close(conn_fd);
    }

    return 0;
}
