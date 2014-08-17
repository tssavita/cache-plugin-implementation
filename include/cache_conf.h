#ifndef _CACHE_CONF_H
#define _CACHE_CONF_H

#include <monkey/mk_api.h>

struct cache_config {
    int max_file_size;
    int expiry_time;
    struct mk_list *mime_types_list;
};

struct cache_config *cache_conf;

void cache_config_file_read(char *path);

#endif
