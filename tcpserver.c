#include "unp.h"
#include "log.h"
#include "conf.h"
#include "http.h"
#include "map.h"

#define koko fprintf(stderr, "ライン %d: %s() ここまで\n", __LINE__, __func__)

void http_data_to_html_table(http_data *hp, char *table, size_t len)
{
    snprintf(table, len,
            "<tr><th>%s</th><td>%s</td></tr>",
            http_method_str(hp->parser->method),
            string_cstr(hp->url));

    const char *key;
    map_str_t *m = hp->head;
    map_iter_t iter = map_iter(m);

    while ((key = map_next(m, &iter))) {
        size_t n = strlen(table);
        snprintf(table + n, len - n,
                "<tr><th>%s</th><td>%s</td></tr>",
                key, *map_get(m, key));
    }

    if (!string_empty(hp->body)) {
        size_t n = strlen(table);
        snprintf(table + n, len - n,
                "<tr><th>%s</th><td><pre>%s</pre></td></tr>",
                "BODY", string_cstr(hp->body));
    }

}

void hello(int conn_fd)
{
    char request[MAXLINE];
    char table[MAXLINE * 8];
    char buff[MAXLINE];
    char buf[MAXLINE];
    time_t now;
    struct tm *tm;
    ssize_t n;
    http_data *hp = http_data_new();

    while ((n = readline(conn_fd, request, sizeof(request))) > 0) {
        http_parser_execute(hp->parser, hp->settings, request, n);
        if (hp->state >= HEADER_END) {
            break;
        }
    }
    http_data_to_html_table(hp, table, sizeof(table));
    http_data_free(hp);

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
             "<table><h2>HTTP 请求</h2>%s</table>"
             "</body>"
             "</html>",
             buff, table);

    snprintf(buff, sizeof(buff),
             "HTTP/1.1 200 OK\r\n"
             "Server: Time\r\n"
             "Date: %s"
             "Content-Type: text/html;charset=UTF-8\r\n"
             "Content-Length: %ld\r\n"
             "\r\n%s\r\n\r\n",
             ctime(&now), strlen(buf) + 4, buf);

    n = writen(conn_fd, buff, strlen(buff));
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

    configuration *conf = config_new();
    config_init(conf);

    if (argc < 2 || (port = atoi(argv[1])) < 1) {
        if (!config_get_int(conf, "http", "listen", &port)) {
            port = 8081;
        }
    }

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
    log_info("服务器启动在 %d 端口", port);

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
            close(listen_fd);
            hello(conn_fd);
            config_free(conf);
            exit(0);
        } else if (pid < 0) {
            log_error("创建新进程失败");
            exit(1);
        }
        close(conn_fd);
    }

    return 0;
}
