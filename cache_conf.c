#include "include/cache_conf.h"
#include "include/cache.h"

#include <monkey/mk_api.h>
#include <monkey/mk_config.h>

void cache_config_file_read(char *path) {
    unsigned long len;
    struct mk_config *conf;
    struct mk_config_section *section;
    char *default_file = NULL;

    mk_api->str_build(&default_file, &len, "%scache.conf", path);
    conf = mk_api->config_create(default_file);
    section = mk_api->config_section_get(conf, "CACHE_CONF");

    if (!section) {
        mk_err("Could not file section CACHE_CONF in the configuration file.");
        exit(EXIT_FAILURE);
    }

    cache_conf = mk_api->mem_alloc(sizeof(struct cache_config));

    cache_conf->max_file_size = (size_t) mk_api->config_section_getval(section, "MaxFileSize", MK_CONFIG_VAL_NUM);

    if (cache_conf->max_file_size <= 0) 
        mk_err("MaxFileSize cannot be zero");
    else
        cache_conf->max_file_size *= 1024;

    cache_conf->expiry_time = (size_t) mk_api->config_section_getval(section, "ExpiryTime", MK_CONFIG_VAL_NUM);

    if (config->timeout < 1) 
        mk_err("Timeout should be set");


    cache_conf->mime_types_list = mk_api->config_section_getval(section, "MimeTypesList", MK_CONFIG_VAL_LIST);

    mk_api->mem_free(default_file);
}
