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

#include <string.h>

#include "MKPlugin.h"
#include "mk_http.h"
#include "cache.h"
#include "cache_conf.h"
#include "cache_operation.h"
#include "constants.h"
#include "errno.h"

MONKEY_PLUGIN("cache",             /* shortname */
              "Monkey Caching plugin",             /* name */
              VERSION,             /* version */
              MK_PLUGIN_STAGE_30 );
//              MK_PLUGIN_CORE_PRCTX | MK_PLUGIN_CORE_THCTX); /* hooks */

/* Init plugin */
int _mkp_init(struct plugin_api **api, char *confdir)
{
    int ret;
    (void) confdir;
    mk_api = *api;

    PLUGIN_TRACE("Initializing");

    /* Start the plugin configuration */
    ret = proxy_conf_init(confdir);
    if (ret != 0) {
        mk_err("Proxy configuration failed. Aborting.");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_init(&mutex_proxy_backend, (pthread_mutexattr_t *) NULL);
    mk_list_init(&proxy_channels);
    PLUGIN_TRACE ("Initializing cache before cache_init ()\n");
    cache_init();
    PLUGIN_TRACE ("Initializing cache\n");

    return 0;
}

/* Exit plugin */
void _mkp_exit()
{
    PLUGIN_TRACE("Exiting");
    cache_destroy();
}

/* Content handler: the real proxy stuff happens here */
int _mkp_stage_30(struct plugin *plugin, struct client_session *cs,
                  struct session_request *sr)
{
    (void) plugin;
    (void) cs;
    char uri[MAX_URI_LEN];
    char vhost[MAX_VHOST_LEN];
    char static_uri[100] = "/home/savita/june_29/htdocs";
    char full_path[MAX_PATH_LEN];

    struct file_t *file;

    int uri_len = sr->uri_processed.len > MAX_URI_LEN ?
        MAX_URI_LEN : sr->uri_processed.len;

    int vhost_len = sr->host_alias->len > MAX_VHOST_LEN ?
        MAX_VHOST_LEN : sr->host_alias->len;

    strncpy (uri, sr->uri_processed.data, uri_len);
    strncpy (vhost, sr->host_alias->name, vhost_len);

    strcat (full_path, static_uri);
    strcat (full_path, uri);
    full_path[MAX_PATH_LEN - 1] = '\0';

    PLUGIN_TRACE ("sr->method %s", sr->method);

    /*if (sr->method == HTTP_METHOD_GET) {*/
        //file = cache_lookup_file (uri);
//        if (!file) {
            file = cache_add_file (full_path, uri);
            PLUGIN_TRACE ("file = %s", file);
  //      }
/*    }*/

/*    PLUGIN_TRACE("Looking up resource requested (which is, %s, %s, %s)", uri, vhost, full_path);

    cache_add_file (full_path, uri);*/

    if (!file) {
        return MK_PLUGIN_RET_NOT_ME;
    }

//    cache_lookup_file (sr->real_path.data);
//    printf("Caching plugin under construction\n");

    memset (uri, '\0', sizeof(uri));
    memset (vhost, '\0', sizeof(vhost));
    memset (full_path, '\0', sizeof(full_path));

    return MK_PLUGIN_RET_NOT_ME;
}

/*void _mkp_event_error () {
}

void _mkp_event_timeout () {
}*/
