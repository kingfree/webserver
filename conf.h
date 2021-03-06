#ifndef _CONF_H_
#define _CONF_H_

typedef struct {
    char inited;
    int port;
    char *root;
    char *server_name;
    char *index;
    char *not_found;
} configuration;

configuration global_settings;

void conf_init();
void conf_load(const char *filename);

#endif
