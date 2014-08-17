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
#include "include/constants.h"
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

    PLUGIN_TRACE("Initializing");
    mk_mimetype_read_config();

    cache_process_init();

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
    
    config = conf;

    return 0;
}

void _mkp_core_thctx () {

    PLUGIN_TRACE ("Starting thread hooks for caching plugin");

    cache_thread_init ();
    cache_stats_thread_init ();
}

int cJSON_stats (struct client_session *cs, struct session_request *sr) {
    mk_api->header_set_http_status (sr, MK_HTTP_OK);

    cJSON *root, *reqs;
    char *msg_to_send;
    char *mime_string = "type.json";

    mk_ptr_t *type_ptr = malloc(sizeof(mk_ptr_t));
    memset(type_ptr, 0, sizeof(mk_ptr_t));
    mk_ptr_set(type_ptr, mime_string);

    root = cJSON_CreateObject();
    reqs = cJSON_CreateObject();
    
    cJSON_AddItemToObject(root, "requests", reqs);
    cJSON_AddNumberToObject(reqs, "finished_per_sec", global_stats.reqs_psec);

    msg_to_send = cJSON_Print(root);
    sr->headers.content_length = strlen(msg_to_send);
    sr->headers.real_length = strlen(msg_to_send);
    struct mimetype *mime = malloc(sizeof(struct mimetype));
    memset(mime, 0, sizeof(struct mimetype));
    mime = mk_mimetype_find(type_ptr);

    sr->headers.content_type = mime->type;
    mk_api->header_send(cs->socket, cs, sr);
    mk_api->socket_send(cs->socket, msg_to_send, strlen(msg_to_send));

    cJSON_Delete(root);
    free(msg_to_send);
    return MK_PLUGIN_RET_END;
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

    cache_stats_new();

    int uri_len = sr->uri_processed.len > MAX_URI_LEN ?
        MAX_URI_LEN : sr->uri_processed.len;

    int path_len = sr->real_path.len > MAX_PATH_LEN ?
        MAX_PATH_LEN : sr->real_path.len;

    memcpy(uri, sr->uri_processed.data, uri_len);
    memcpy (path, sr->real_path.data, path_len);
    uri[uri_len] = '\0';
    path[path_len] = '\0';
            PLUGIN_TRACE("path in stats - %s", path);

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

            int pluglen = strlen(PLUGDIR);
            memcpy(path, PLUGDIR, pluglen);

            int level_1 = strlen(path);
            int pluglen2 = pluglen + strlen(UI_URL);
            memcpy(path + level_1, UI_URL, pluglen2);

            int level_2 = strlen(path);
            int pluglen3 = pluglen2 + strlen(UI_DIR);
            memcpy(path + level_2, UI_DIR, pluglen3);

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
    PLUGIN_TRACE ("path = %s, %d", path, strlen(path));
            
    if (!file_found) 
        file = cache_add_file (path, uri);
    
    if (!file) {
    PLUGIN_TRACE ("path = %s, %d", path, strlen(path));
        return MK_PLUGIN_RET_NOT_ME;
    }

    mk_api->header_set_http_status (sr, MK_HTTP_OK);
    sr->headers.content_length = file->content.len;
    sr->headers.real_length = file->content.len;
    PLUGIN_TRACE ("path = %s, %d", path, strlen(path));

    /* Using the mime type module of Monkey server to find 
     out the content type of the requested file and fill up
     the content_type field in the request header.*/

/*    mk_ptr_t *file_name = malloc(sizeof(mk_ptr_t));
    memset(file_name, 0, sizeof(mk_ptr_t));
    mk_ptr_set(file_name, path);

    struct mimetype *mime = mk_api->mem_alloc_z(sizeof(mk_ptr_t));
    mime = mk_mimetype_find(file_name);
    if (mime == NULL) { 
    PLUGIN_TRACE ("path = %s, %d", path, strlen(path));
        mime = mimetype_default;
    }
    sr->headers.content_type = mime->type;*/


    struct mk_list *head;
    struct mk_string_line *entry;
    char *ext = file_ext(path);
    struct mimetype *mime = mk_api->mem_alloc_z(sizeof(mk_ptr_t));
    mk_ptr_t *type_ptr = malloc(sizeof(mk_ptr_t));
    memset(type_ptr, 0, sizeof(mk_ptr_t));
    PLUGIN_TRACE ("path = %s, %d, %s, %d, %d", path, strlen(path), ext, cache_conf->max_file_size, cache_conf->expiry_time);

    if (cache_conf->mime_types_list)
        PLUGIN_TRACE("List is null");
    
    mk_list_foreach(head, cache_conf->mime_types_list) {
        entry = mk_list_entry(head, struct mk_string_line, _head);
    PLUGIN_TRACE ("path = %s, %d, %s, %s", path, strlen(path), entry->val, ext);

        if (strcmp(ext, entry->val) == 0) 
            break;
    }
    PLUGIN_TRACE ("path = %s, %d", path, strlen(path));

    mime = mk_mimetype_lookup(ext);
    PLUGIN_TRACE ("path = %s, %d", path, strlen(path));
    sr->headers.content_type = mime->type;

    PLUGIN_TRACE ("file = %s", file->content.data);
    
    sr->headers.sent = MK_TRUE;
    mk_api->header_send(cs->socket, cs, sr);
    mk_api->socket_send(cs->socket, file->content.data, file->content.len);

    memset (uri, '\0', sizeof(uri));
    memset (path, '\0', sizeof(path));
    PLUGIN_TRACE ("file = %s", file->content.data);

    return MK_PLUGIN_RET_END;
}

