#ifndef _HTTP_H_
#define _HTTP_H_

#include "map.h"
#include "http_parser.h"

typedef struct {
    struct http_parser parser;
    map_str_t head;
    char *body;
} http_data;


#endif
