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

#ifndef _REQUEST_H
#define _REQUEST_H

#include "cache_operation.h"
#include "utils.h"
#include "MKPlugin.h"

struct request_t {
    int socket;
    long bytes_to_send;
    off_t bytes_offset;

    struct file_t *file;

    struct mk_list _head;
};

extern pthread_key_t request_list;

void request_process_init ();

void request_thread_init ();

struct request_t *request_new ();

int request_fill (struct request_t *req);

void request_finish ();

void request_delete (struct request_t *req);

#endif
