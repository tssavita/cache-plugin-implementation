/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Monkey HTTP Server
 *  ==================
 *  Copyright 2001-2014 Monkey Software LLC <eduardo@monkey.io>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

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
