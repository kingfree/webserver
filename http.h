#ifndef _HTTP_H_
#define _HTTP_H_

#include "map.h"
#include "string.h"
#include "http_parser.h"

typedef enum {
    NONE, BEGIN, URL, STATUS, FIELD, VALUE, HEADER_END, BODY, END, CHUNK
} parser_state;

typedef struct {
    struct http_parser *parser;
    parser_state state;
    map_str_t *head;
    string *url;
    string *body;
    struct http_parser_settings *settings;
    char *_last_key;
} http_data;

http_data *http_data_new();
void http_data_free(http_data *hp);

#endif
