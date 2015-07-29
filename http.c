#include "http.h"
#include "map.h"
#include "string.h"

#include <stdio.h>
#include <stdlib.h>

#define koko fprintf(stderr, "ライン %d: %s() ここまで\n", __LINE__, __func__)

int request_on_message_begin(http_parser *);
int request_on_url(http_parser *, const char *at, size_t length);
int request_on_status(http_parser *, const char *at, size_t length);
int request_on_header_field(http_parser *, const char *at, size_t length);
int request_on_header_value(http_parser *, const char *at, size_t length);
int request_on_headers_complete(http_parser *);
int request_on_body(http_parser *, const char *at, size_t length);
int request_on_message_complete(http_parser *);
int request_on_chunk_header(http_parser *);
int request_on_chunk_complete(http_parser *);

struct http_parser_settings request_settings = {
    .on_message_begin = request_on_message_begin,
    .on_url = request_on_url,
    .on_status = request_on_status,
    .on_header_field = request_on_header_field,
    .on_header_value = request_on_header_value,
    .on_headers_complete = request_on_headers_complete,
    .on_body = request_on_body,
    .on_message_complete = request_on_message_complete,
    .on_chunk_header = request_on_chunk_header,
    .on_chunk_complete = request_on_chunk_complete
};

http_data *http_data_new()
{
    http_data *hp = malloc(sizeof(http_data));

    hp->parser = malloc(sizeof(struct http_parser));
    http_parser_init(hp->parser, HTTP_REQUEST);
    hp->parser->data = hp;

    hp->head = malloc(sizeof(map_str_t));
    map_init(hp->head);

    hp->url = string_new();

    hp->body = string_new();

    hp->settings = &request_settings;

    hp->state = NONE;

    return hp;
}

void http_data_free(http_data *hp)
{
    if (!hp) return;
    free(hp->parser);
    //map_deinit(hp->head);
    string_free(hp->url);
    string_free(hp->body);
    free(hp);
    hp = NULL;
}

int request_on_message_begin(http_parser *p)
{
    http_data *hp = (http_data *)p->data;
    hp->state = BEGIN;
    return 0;
}

int request_on_url(http_parser *p, const char *at, size_t length)
{
    http_data *hp = (http_data *)p->data;
    hp->state = URL;
    string_ncat(hp->url, at, length);
    return 0;
}

int request_on_status(http_parser *p, const char *at, size_t length)
{
    http_data *hp = (http_data *)p->data;
    hp->state = STATUS;
    return 0;
}

int request_on_header_field(http_parser *p, const char *at, size_t length)
{
    http_data *hp = (http_data *)p->data;
    hp->state = FIELD;
    hp->_last_key = strndup(at, length);
    return 0;
}

int request_on_header_value(http_parser *p, const char *at, size_t length)
{
    http_data *hp = (http_data *)p->data;
    hp->state = VALUE;
    char *value = strndup(at, length);
    map_set(hp->head, hp->_last_key, value);
    return 0;
}

int request_on_headers_complete(http_parser *p)
{
    http_data *hp = (http_data *)p->data;
    hp->state = HEADER_END;
    return 0;
}

int request_on_body(http_parser *p, const char *at, size_t length)
{
    http_data *hp = (http_data *)p->data;
    hp->state = BODY;
    string_ncat(hp->body, at, length);
    return 0;
}

int request_on_message_complete(http_parser *p)
{
    http_data *hp = (http_data *)p->data;
    hp->state = END;
    return 0;
}

int request_on_chunk_header(http_parser *p)
{
    http_data *hp = (http_data *)p->data;
    hp->state = CHUNK;
    return 0;
}

int request_on_chunk_complete(http_parser *p)
{
    http_data *hp = (http_data *)p->data;
    hp->state = END;
    return 0;
}


