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

#ifndef _HASH_TABLE_H_
#define _HASH_TABLE_H_

#include <stdlib.h>
#include <stdio.h>

#define TABLE_SIZE 50

/* Structure for the key to be inserted in the hash table. */
struct node_t {
    const char* key;
    void *data;
    struct node_t *next;
};

/* Structure for the hash table. */
struct table_t {
    struct node_t **table_list;
    int table_size;
};

struct table_t *table_create ();

int table_insert (struct table_t *table, const char *key, void *data);

void *table_file_info(struct table_t *table, void *result);

void *table_lookup (struct table_t *table, const char *key);

void *table_delete (struct table_t *table, const char *key);

void table_destroy ();

#endif
