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

#include <monkey/mk_info.h>
#include <monkey/mk_plugin.h>
#include <monkey/mk_http.h>
#include <monkey/mk_mimetype.h>
#include <monkey/mk_memory.h>
#include <monkey/mk_config.h>
#include <monkey/mk_file.h>
#include <monkey/monkey.h>
#include <monkey/mk_string.h>
#include <monkey/mk_utils.h>
#include <monkey/mk_request.h>
#include <monkey/mk_api.h>
#include <monkey/mk_list.h>

//#include "cache.h"
#include "include/cache_conf.h"
#include "include/cJSON.h"
#include "include/cache_operation.h"
#include "include/cache_stats.h"
#include "include/stats_timer.h"
#include "include/hash_func.h"
#include "include/hash_table.h"
//#include "include/constants.h"
#include "include/utils.h"

MONKEY_PLUGIN("cache",             /* shortname */
              "Monkey Caching plugin",             /* name */
              VERSION,             /* version */
              MK_PLUGIN_STAGE_30 | MK_PLUGIN_CORE_PRCTX | MK_PLUGIN_CORE_THCTX); /* hooks */

/* Init plugin */
int _mkp_init(struct plugin_api **api, char *confdir)
{
    (void) confdir;
    mk_api = *api;
    cache_config_file_read(confdir);
    mk_mimetype_read_config();

    PLUGIN_TRACE("Initializing");

    return 0;
}

/* Exit plugin */
void _mkp_exit()
{
    PLUGIN_TRACE("Exiting");
//    cache_destroy ();
}

struct server_config *config;

int _mkp_core_prctx (struct server_config *conf) {

    PLUGIN_TRACE ("Starting process hooks for caching plugin");

    cache_process_init ();
    cache_stats_process_init ();
    stats_timer_process_init();
    
    config = conf;

    return 0;
}

void _mkp_core_thctx () {

    PLUGIN_TRACE ("Starting thread hooks for caching plugin");

    cache_thread_init ();
    cache_stats_thread_init ();
    stats_timer_thread_init();
}

int _mkp_event_read(int fd) {
    if (fd == stats_timer_get_fd()) {
        stats_timer_read();
        return  MK_PLUGIN_RET_EVENT_OWNED;
    }

    return MK_PLUGIN_RET_EVENT_NEXT;
}

int cJSON_stats (struct client_session *cs, struct session_request *sr) {
    mk_api->header_set_http_status (sr, MK_HTTP_OK);

    cJSON *root, *reqs, *files;
    char *msg_to_send;
    const mk_ptr_t json_type = mk_ptr_init("application/json\r\n");

    root = cJSON_CreateObject();
    reqs = cJSON_CreateObject();

    PLUGIN_TRACE("fin req = %d", global_stats.reqs_psec);
    
    cJSON_AddItemToObject(root, "requests", reqs);
    cJSON_AddNumberToObject(reqs, "finished_per_sec", global_stats.reqs_psec);

    files = cJSON_CreateObject();

    cJSON_AddItemToObject(root, "files", files);
    table_file_info(hash_table, files);

    msg_to_send = cJSON_Print(root);
    sr->headers.content_length = strlen(msg_to_send);
    sr->headers.real_length = strlen(msg_to_send);

    sr->headers.content_type = json_type;
//    sr->headers.content_type.len = strlen(mime_string);
    mk_api->header_send(cs->socket, cs, sr);
    mk_api->socket_send(cs->socket, msg_to_send, strlen(msg_to_send));

    cJSON_Delete(root);
    free(msg_to_send);
    return MK_PLUGIN_RET_END;
}

void *cJSON_stats_file(const char *key, void *val, void *result) {
    cJSON *files = result;
    cJSON *file = cJSON_CreateObject();

    (void) key;

    struct file_t *file_content = val;
    cJSON_AddItemToArray(files, file);
    cJSON_AddStringToObject(file, "name", file_content->name);
    cJSON_AddNumberToObject(file, "size", file_content->size);
    cJSON_AddNumberToObject(file, "access_count", file_content->count);
    return files;
}

/* Content handler: the request is handled here. */
int _mkp_stage_30(struct plugin *plugin, struct client_session *cs,
                  struct session_request *sr) {

    (void) plugin;
    (void) cs;
    char uri[MAX_URI_LEN];
    char path[MAX_PATH_LEN];
    int file_found = 0;

    struct file_t *file;

    int uri_len = sr->uri_processed.len > MAX_URI_LEN ?
        MAX_URI_LEN : sr->uri_processed.len;

    int path_len = sr->real_path.len > MAX_PATH_LEN ?
        MAX_PATH_LEN : sr->real_path.len;

    memcpy(uri, sr->uri_processed.data, uri_len);
    uri[uri_len] = '\0';

    memcpy (path, sr->real_path.data, path_len);
    path[path_len] = '\0';
    PLUGIN_TRACE ("path = %s, %d", path, strlen(path));


//    int cache_flag = 0;
//    struct mk_list *head;
    char *ext = file_ext(path);

    if (!ext)
        ext = "mime not found";

    if (uri_len > 6 && memcmp(uri, UI_URL, UI_URL_LEN) == 0) {

        mk_ptr_t url;
        url.data = uri + UI_URL_LEN;
        url.len = uri_len - UI_URL_LEN;
        
        if (url.len == 6 && memcmp(url.data, "/stats", 6) == 0) {
            PLUGIN_TRACE("path in stats - %s", path);
            return cJSON_stats(cs, sr);
        }

        if (url.len == 11 && memcmp(url.data, "/index.html", 11) == 0) {
            ext = "html";

            path_len = strlen(PLUGIN_UI);
            memcpy(path, PLUGIN_UI, strlen(PLUGIN_UI));
            path[path_len] = '\0';

            PLUGIN_TRACE("path in stats - %s", path);
            file = cache_add_file(path, url.data, ext);
                
            if (file != NULL) {
                file_found = 1;
            PLUGIN_TRACE("file found or not - %d", file_found);
            }
        }
    }
    PLUGIN_TRACE ("path = %s, %d", path, strlen(path));

    if (!file_found ) 
        file = cache_add_file (path, uri, ext);

    if (!file) 
        return MK_PLUGIN_RET_NOT_ME;
    else 
        cache_stats_new();

    mk_api->header_set_http_status (sr, MK_HTTP_OK);
    sr->headers.content_length = file->content.len;
    sr->headers.real_length = file->content.len;
    sr->headers.content_type = file->type;
    sr->headers.sent = MK_TRUE;
    mk_api->socket_send(cs->socket, file->content.data, file->content.len);

    if ((config->max_keep_alive_request - cs->counter_connections) <= 0) 
        mk_api->header_send(cs->socket, cs, sr);

    PLUGIN_TRACE("(config->max_keep_alive_request = %d", config->max_keep_alive_request);
    PLUGIN_TRACE("cs->counter_connections = %d", cs->counter_connections);
    return MK_PLUGIN_RET_END;
}
