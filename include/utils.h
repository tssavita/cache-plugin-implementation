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

#ifndef _UTILS_H
#define _UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <monkey/mk_api.h>
#include <monkey/mk_config.h>
#include <monkey/mk_plugin.h>

#define handle_error(msg) \
        do { perror (msg); \
            exit(EXIT_FAILURE); \
        } while (0)

#define MODE(z) ((st.st_mode &(z)) == z)

typedef enum {false, true} bool;

#define REQ_LIST_LIMIT 24

#define MAX_URI_LEN 1024
#define MAX_PATH_LEN 1024 
#define MAX_VHOST_LEN 1024

#define MAX_LEN_MIME_NAME 20
#define MAX_LEN_MIME_TYPE 20

#define UI_URL "/cache"
#define UI_URL_LEN 6

#define UI_DIR "/conf/ui_2/index.html"
#define UI_DIR_LEN 21

#define PLUGIN_UI1 PLUGDIR UI_URL
#define PLUGIN_UI PLUGIN_UI1 UI_DIR

#define MIME_CONF "/conf/mime.conf"
#define MIME_CONF_LEN 5 

char *file_ext(char *filename);

#endif
