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

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "mk_info.h"
#include "MKPlugin.h"
#include "mk_http.h"
#include "errno.h"
#include "mk_mimetype.h"
#include "mk_memory.h"
#include "mk_config.h"
#include "mk_file.h"
#include "monkey.h"
#include "mk_string.h"
#include "mk_utils.h"
#include "mk_request.h"

//#include "cache.h"
#include "include/cJSON.h"
#include "include/cache_conf.h"
#include "include/cache_operation.h"
//#include "include/cache_request.h"
#include "include/cache_stats.h"
//#include "include/request.h"
#include "include/constants.h"
//#include "include/file_type.h"

MONKEY_PLUGIN("cache",             /* shortname */
              "Monkey Caching plugin",             /* name */
              VERSION,             /* version */
              MK_PLUGIN_STAGE_30 | MK_PLUGIN_CORE_PRCTX | MK_PLUGIN_CORE_THCTX); /* hooks */

char conf_path[MAX_PATH_LEN];
int conf_path_len;

/* Init plugin */
int _mkp_init(struct plugin_api **api, char *confdir)
{
    int ret;
    strncpy(conf_path, confdir, MAX_PATH_LEN);
    conf_path_len = strlen(conf_path);

    (void) confdir;
    mk_api = *api;
    PLUGIN_TRACE("conf_path - %s", conf_path);
    PLUGIN_TRACE("conf_dir - %s", confdir);

    PLUGIN_TRACE("Initializing");

    /* Start the plugin configuration */
    ret = proxy_conf_init(confdir);
    if (ret != 0) {
        mk_err("Proxy configuration failed. Aborting.");
        exit(EXIT_FAILURE);
    }

    mk_mimetype_read_config();

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
    //request_finish ();
    PLUGIN_TRACE("Exiting");
//    cache_destroy ();
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

int cJSON_stats (struct client_session *cs, struct session_request *sr) {
    mk_api->header_set_http_status (sr, MK_HTTP_OK);

    PLUGIN_TRACE("Step1");

    cJSON *root, *reqs;
    char *msg_to_send;
    char *mime_string = "type.json";
    PLUGIN_TRACE("Step1");

    mk_ptr_t *type_ptr;
    type_ptr = mk_api->mem_alloc_z(sizeof(mk_ptr_t));
    mk_ptr_t_set(type_ptr, mime_string);

    root = cJSON_CreateObject();
    reqs = cJSON_CreateObject();
    
    cJSON_AddItemToObject(root, "requests", reqs);
    cJSON_AddNumberToObject(reqs, "finished_per_sec", global_stats.reqs_psec);

    msg_to_send = cJSON_Print(root);
    sr->headers.content_length = strlen(msg_to_send);
    sr->headers.real_length = strlen(msg_to_send);
    struct mimetype *mime = mk_api->mem_alloc_z(sizeof(struct mimetype));
    mime = mk_mimetype_find(type_ptr);

    if (!mime)
        PLUGIN_TRACE("mime is null");

    sr->headers.content_type = mime->type;
    mk_api->header_send(cs->socket, cs, sr);
    mk_api->socket_send(cs->socket, msg_to_send, strlen(msg_to_send));

    cJSON_Delete(root);
    free(msg_to_send);
    return MK_PLUGIN_RET_END;
}

/* Content handler: the real proxy stuff happens here */
int _mkp_stage_30(struct plugin *plugin, struct client_session *cs,
                  struct session_request *sr) {

    (void) plugin;
    (void) cs;
    char uri[MAX_URI_LEN];
    char path[MAX_PATH_LEN];
    int file_found = 0;

    struct file_t *file;

    cache_stats_new();

    int uri_len = sr->uri_processed.len > MAX_URI_LEN ?
        MAX_URI_LEN : sr->uri_processed.len;

    int path_len = sr->real_path.len > MAX_PATH_LEN ?
        MAX_PATH_LEN : sr->real_path.len;

    strncpy (uri, sr->uri_processed.data, uri_len);
    strncpy (path, sr->real_path.data, path_len);
    uri[uri_len] = '\0';
    path[path_len] = '\0';
    PLUGIN_TRACE("path = %s", path);

    if (uri_len > 6 && memcmp(uri, UI_URL, UI_URL_LEN) == 0) {

        mk_ptr_t url;
        url.data = uri + UI_URL_LEN;
        url.len = uri_len - UI_URL_LEN;
        
        if (url.len == 6 && memcmp(url.data, "/stats", 6) == 0) {
            PLUGIN_TRACE("path in stats - %s", path);
            return cJSON_stats(cs, sr);
        }

        if (url.len == 11 && memcmp(url.data, "/index.html", 11) == 0) {
            memset (path, '\0', sizeof(path));

            PLUGIN_TRACE("path in stats - %s", PLUGDIR);
            memcpy(path, PLUGDIR, MAX_PATH_LEN);
            int level_1 = strlen(path);
            PLUGIN_TRACE("path in stats - %s", path);
            memcpy(path + level_1, UI_URL, MAX_PATH_LEN);
            int level_2 = strlen(path);
            PLUGIN_TRACE("path in stats - %s", path);
            memcpy(path + level_2, UI_DIR, MAX_PATH_LEN);
            PLUGIN_TRACE("path in stats - %s", path);

            path_len = strlen(path);
            
            path[path_len] = '\0';

            PLUGIN_TRACE("path in stats - %s", path);
            file = cache_add_file(path, url.data);
            if (file != NULL) {
                file_found = 1;
            PLUGIN_TRACE("file found or not - %d", file_found);
            }
        }
    }
    PLUGIN_TRACE ("path = %s", path);
            
    if (!file_found) 
        file = cache_add_file (path, uri);
    
    if (!file) 
        return MK_PLUGIN_RET_NOT_ME;

    mk_api->header_set_http_status (sr, MK_HTTP_OK);
    sr->headers.content_length = file->content.len;
//    sr->headers.real_length = file->size;

    /* Using the mime type module of Monkey server to find 
     out the content type of the requested file and fill up
     the content_type field in the request header.*/

    mk_ptr_t *file_name = mk_api->mem_alloc_z(sizeof(mk_ptr_t));
    mk_ptr_t_set(file_name, path);

    struct mimetype *mime = mk_api->mem_alloc_z(sizeof(mk_ptr_t));;
    mime = mk_mimetype_find(file_name);
    sr->headers.content_type = mime->type;
    //sr->headers.content_type.data = "text/html\n";
//    sr->headers.content_type.len = 10;
//    sr->headers.content_encoding.data = "UTF-8";
    
    mk_api->header_send(cs->socket, cs, sr);
    mk_api->socket_send(cs->socket, file->content.data, file->content.len);
    PLUGIN_TRACE ("file = %s", file->content.data);

    sr->headers.sent = MK_TRUE;

    memset (uri, '\0', sizeof(uri));
    memset (path, '\0', sizeof(path));

    return MK_PLUGIN_RET_END;
}

int _mkp_event_error (int socket_fd) {
    
    PLUGIN_TRACE("[FD %i] An error occured", socket_fd);
    /*cache_request_delete (socket_fd);*/

    return MK_PLUGIN_RET_EVENT_NEXT;
}

int _mkp_event_timeout (int socket_fd) {
    
    PLUGIN_TRACE("[FD %i] Request timed out", socket_fd);
    /*cache_request_delete (socket_fd);*/
    
    return MK_PLUGIN_RET_EVENT_NEXT;
}
