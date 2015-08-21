#include "unp.h"
#include "log.h"
#include "conf.h"
#include "http.h"
#include "map.h"

#define koko fprintf(stderr, "ライン %d: %s() ここまで\n", __LINE__, __func__)

configuration *conf;

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

char *get_file_mime(const char *filename)
{
    char *p = strrchr(filename, '.');
    if (!p) return "text/plain";
    p++;
    log_info("扩展名: %s", p);
    if (strcmp(p, "gif") == 0) return "image/gif";
    if (strcmp(p, "ico") == 0) return "image/x-icon";
    if (strcmp(p, "png") == 0) return "image/png";
    if (strcmp(p, "html") == 0) return "text/html;charset=UTF-8";
    if (strcmp(p, "php") == 0) return "text/html;charset=UTF-8";
    return "text/plain";
}

int get_file(const char *filename, char *buf, size_t maxlen)
{
    int fd;
    long n;
    string *path = string_new();

    string_cat(path, global_settings.root);
    string_cat(path, filename);
    log_info("获取文件: %s", string_cstr(path));
    
    fd = open(string_cstr(path), O_RDONLY);
    if (fd < 0) return 0;

    n = readn(fd, buf, maxlen);
    close(fd);

    if (n < 0) return 0;
    buf[n] = '\0';
    return n;
}

char *get_file_name(string *url)
{
    string *res = string_new();
    char *s = strdup(string_cstr(url));
    char *pq = strchr(s, '?');
    if (pq && pq != s) *pq = '\0';
    char *pd = strrchr(s, '/');
    long l = strlen(s);
    string_ncat(res, s, l);
    if (pd - s + 1 == l) string_cat(res, global_settings.index);
    return string_cstr(res);
}

void hello(int conn_fd)
{
    char request[MAXLINE];
    char table[MAXLINE * 8];
    char buff[MAXLINE];
    // char buf[MAXLINE];
    time_t now;
    // struct tm *tm;
    long n;
    http_data *hp = http_data_new();

    while ((n = readline(conn_fd, request, sizeof(request))) > 0) {
        log_info("读取一行: %s", request);
        http_parser_execute(hp->parser, hp->settings, request, n);
        if (hp->state >= END) {
            break;
        }
    }
    log_info("URL: %s", string_cstr(hp->url));

    http_data_to_html_table(hp, table, sizeof(table));

    char *filename = get_file_name(hp->url);

    n = get_file(filename, request, sizeof(request));

    http_data_free(hp);

    /* now = time(NULL);
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
             "<h2>文件内容</h2>"
             "<div>%s</div>"
             "</body>"
             "</html>",
             buff, table, request);*/

    snprintf(buff, sizeof(buff),
             "HTTP/1.1 200 OK\r\n"
             "Server: %s\r\n"
             "Date: %s"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\r\n"
             "\r\n",
             global_settings.server_name, ctime(&now), get_file_mime(filename), n); 

    writen(conn_fd, buff, strlen(buff));
    writen(conn_fd, request, n);
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

    conf_init();

    if (argc < 2 || (port = atoi(argv[1])) < 1) {
        port = global_settings.port;
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
            exit(0);
        } else if (pid < 0) {
            log_error("创建新进程失败");
            exit(1);
        }
        close(conn_fd);
    }

    return 0;
}
