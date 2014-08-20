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

#ifndef _CACHE_OPERATION_H_
#define _CACHE_OPERATION_H_

#include <stdio.h>
#include <stdlib.h>
#include "include/hash_table.h"
#include <monkey/mk_memory.h>

#define MAX_LENGTH_NAME 1024

struct table_t *hash_table;
struct heap_t *heap;

/* Structure for the file to be inserted 
   in the min heap. */
struct file_t {

    char name[MAX_LENGTH_NAME];
    int fd;
    mk_ptr_t content;
    size_t size;
    int count;
    time_t mapped_at;
};

void cache_process_init();

void cache_thread_init ();

void file_access_count (struct file_t *file);

struct file_t *cache_add_file (const char *uri, const char *content);

struct file_t *cache_lookup_file (const char *uri);

void cache_unmap_file (struct file_t *file);

void cache_destroy ();

#endif
