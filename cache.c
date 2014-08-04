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
#include "errno.h"
#include "mk_mimetype.h"
#include "mk_memory.h"

//#include "cache.h"
#include "include/cache_conf.h"
#include "include/cache_operation.h"
#include "include/cache_request.h"
#include "include/cache_stats.h"
#include "include/request.h"
#include "include/constants.h"
#include "include/cJSON.h"

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
    cache_process_init();
    PLUGIN_TRACE ("Initializing cache\n");

    return 0;
}

/* Exit plugin */
void _mkp_exit()
{
    request_finish ();
    PLUGIN_TRACE("Exiting");
    cache_destroy ();
}

struct server_config *config;

int _mkp_core_prctx (struct server_config *conf) {

    PLUGIN_TRACE ("Starting process hooks for caching plugin");

    /*cache_request_process_init ();
    request_process_init ();*/
    cache_process_init ();
    cache_stats_process_init ();
    
    config = conf;

    return 0;
}

void _mkp_core_thctx () {

    PLUGIN_TRACE ("Starting thread hooks for caching plugin");

/*    cache_request_thread_init ();
    request_thread_init ();*/
    cache_thread_init ();
    cache_stats_thread_init ();
}

int statistics (struct client_session *cs, struct session_request *sr) {
    mk_api->header_set_http_status (sr, MK_HTTP_OK);

    cJSON *root, *reqs;
    
    root = cJSON_CreateObject();
    reqs = cJSON_CreateObject();
    
    cJSON_AddItemToObject(root, "reqs_psec", reqs);
    cJSON_AddNumberToObject(reqs, "finished requests per sec", global_stats->reqs_psec);
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
    struct mimetype *mime;
    struct request_t *req; /* = cache_request_get(cs->socket);
    if (req) {
        PLUGIN_TRACE ("req is present in list");
        return MK_PLUGIN_RET_CONTINUE;
    }*/

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

    if (!file) 
        return MK_PLUGIN_RET_NOT_ME;

    req = request_new ();
    req->socket = cs->socket;
    req->bytes_offset = 0;
    req->bytes_to_send = file->size;
    req->file = file;
    
    cache_request_add (req);
    request_fill (req);
    
    PLUGIN_TRACE ("Going to set status\n");

    mk_api->header_set_http_status (sr, MK_HTTP_OK);
    sr->headers.content_length = file->size;
    sr->headers.real_length = file->size;


    /* Using the mime type module of Monkey server to find 
     out the content type of the requested file and fill up
     the content_type field in the request header.*/
    mk_ptr_t *file_name;
    mk_ptr_t_set(file_name, full_path);
    mime = mk_mimetype_find(file_name);
    if (!mime)
        mime = mimetype_default;
    sr->headers.content_type = mime->type;

 //   fill_cache_headers (file, cs, sr);
 //   if (config->max_keep_alive_request - cs->counter_connections <= 0)

    mk_api->header_send(cs->socket, cs, sr);
    mk_api->socket_send(cs->socket, file->content.data, file->size);

//    PLUGIN_TRACE ("File content = %s", file->content.data);

    sr->headers.sent = MK_TRUE;

    PLUGIN_TRACE ("file = %s", file);

    memset (uri, '\0', sizeof(uri));
    //memset (path, '\0', sizeof(path));
    memset (vhost, '\0', sizeof(vhost));
    memset (full_path, '\0', sizeof(full_path));

    return MK_PLUGIN_RET_END;
}

int _mkp_event_error (int socket_fd) {
    
    PLUGIN_TRACE ("[FD %i] An error occured", socket_fd);
    cache_request_delete (socket_fd);

    return MK_PLUGIN_RET_EVENT_NEXT;
}

int _mkp_event_timeout (int socket_fd) {
    
    PLUGIN_TRACE ("[FD %i] Request timed out", socket_fd);
    cache_request_delete (socket_fd);
    
    return MK_PLUGIN_RET_EVENT_NEXT;
}
