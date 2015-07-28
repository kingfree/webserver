#include "unp.h"
#include "log.h"

#define DEFAULT_PORT 80

#define koko fprintf(stderr, "ライン %d: %s() ここまで\n", __LINE__, __func__)

void hello(int fd)
{
    char buff[MAXLINE];
    char buf[MAXLINE];
    time_t now;
    struct tm *tm;
    int n;

    now = time(NULL);
    tm = localtime(&now);
    strftime(buff, sizeof(buff), "%G-%m-%d %T", tm);
    snprintf(buf, sizeof(buf),
             "<html>"
             "<head>"
             "<title>主页</title>"
             "</head>"
             "<body>"
             "<h1>Hello, 世界</h1>"
             "<p>当前服务器时间: </p>"
             "<pre>%s</pre>"
             "</body>"
             "</html>",
             buff);

    snprintf(buff, sizeof(buff),
             "HTTP/1.1 200 OK\r\n"
             "Server: Time\r\n"
             "Date: %s"
             "Content-Type: text/html;charset=UTF-8\r\n"
             "Content-Length: %ld\r\n"
             "\r\n%s\r\n",
             ctime(&now), strlen(buf), buf);

    n = send(fd, buff, strlen(buff), 0);
    if (n < 0) {
        log_error("发送数据失败 %d", n);
        exit(1);
    }
    log_info("响应内容: \n%s", buff);
}

void sig_child(int signo)
{
    pid_t pid;
    int stat;

    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        log_info("子进程 %d 正常终止, 信号 %d", pid, signo);
    }
    return;
}

int main(int argc, char *argv[])
{
    int port;
    int listen_fd, conn_fd;
    struct sockaddr_in server, client;
    int res;
    sig_t sig;
    socklen_t socklen;
    int pid;
    char buff[MAXLINE];

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

    sig = signal(SIGCHLD, sig_child);
    if (sig == SIG_ERR) {
        log_error("信号设置失败");
        exit(1);
    }

    for (;;) {
        socklen = sizeof(client);
        conn_fd = accept(listen_fd, (struct sockaddr *)&client, &socklen);
        if (conn_fd < 0) {
            log_error("接收请求失败");
            exit(1);
        }
        log_info("连接来自 %s:%d", inet_ntoa(client.sin_addr),
                 ntohs(client.sin_port));

        pid = fork();
        if (pid == 0) {
            res = recv(conn_fd, buff, sizeof(buff), 0);
            if (res < 0) {
                log_error("接收数据失败");
                exit(1);
            }
            log_info("请求内容: (并不关心)\n%s", buff);

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
