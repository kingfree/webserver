#ifndef _HTTP_H_
#define _HTTP_H_

#include "map.h"
#include "string.h"
#include "http_parser.h"

typedef enum {
    NONE, BEGIN, URL, STATUS, FIELD, VALUE, HEADER_END, BODY, CHUNK, END
} parser_state;

typedef struct {
    struct http_parser *parser;
    struct http_parser_settings *settings;
    parser_state state;
    map_str_t *head;
    string *url;
    string *body;
    char *_last_key;
} http_data;

http_data *http_data_new();
void http_data_free(http_data *hp);

#endif
