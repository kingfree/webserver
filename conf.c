#include "conf.h"
#include "ini.h"

#include <string.h>
#include <stdlib.h>

#define DEFAULT_PORT 80
#define DEFAULT_ROOT "/var/www/"
#define DEFAULT_SERVER_NAME "localhost"

#define DEFAULT_CONF_FILE "conf.ini"


static int conf_handler(void* user, const char* section, const char* name,
                        const char* value)
{
    configuration* pconfig = (configuration*)user;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("http", "listen")) {
        pconfig->port = atoi(value);
    } else if (MATCH("http", "root")) {
        pconfig->root = strdup(value);
    } else if (MATCH("http", "server_name")) {
        pconfig->server_name = strdup(value);
    } else {
        return 0;
    }
    return 1;
}

void conf_init()
{
    global_settings.inited = 1;
    global_settings.port = DEFAULT_PORT;
    global_settings.root = DEFAULT_ROOT;
    global_settings.server_name = DEFAULT_SERVER_NAME;

    conf_load(DEFAULT_CONF_FILE);
}

void conf_load(const char* filename)
{
    if (ini_parse(filename, conf_handler, &global_settings) < 0) {
        global_settings.inited = 0;
        return;
    }
    global_settings.inited = 2;
}
