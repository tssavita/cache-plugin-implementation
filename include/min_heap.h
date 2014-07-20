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

#ifndef _MIN_HEAP_H
#define _MIN_HEAP_H

#define MAX_LENGTH_NAME 1024

struct node_count {
    char name[MAX_LENGTH_NAME];
    int count;
};

struct heap_t {
    int heapsize;
    struct node_count *heap_array;
};

struct heap_t *heap_create();

int heap_insert (struct heap_t *heap, const char *name);

void count_increment (struct heap_t *heap, const char *name);

char *pop (struct heap_t *heap);

void heap_destroy(struct heap_t *heap);

#endif
