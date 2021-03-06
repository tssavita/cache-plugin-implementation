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
#include <stdlib.h>
#include <string.h>
#include <monkey/mk_api.h>

#include "include/min_heap.h"
#include "include/hash_table.h"
#include "include/cache_operation.h"
#include "include/utils.h"
#include <monkey/mk_plugin.h>

/* Function to create the structure (array) for min heap. */

struct heap_t *heap_create() {

    struct heap_t *heap = mk_api->mem_alloc(sizeof(struct heap_t));
    if (!heap)
        return NULL;

    heap->heapsize = 0;

    int space = TABLE_SIZE * (sizeof(struct node_count));
    heap->heap_array  = mk_api->mem_alloc_z(space);
    
    return heap;
}

/* Function to insert file node into the min heap. */

int heap_insert(struct heap_t *heap, const char *name) {

    int name_len = 0;

    if (!heap)
        return false;

    /* Incremented number of elements in min heap. */
    struct node_count *node = mk_api->mem_alloc(sizeof(struct node_count));
    if (!node)
        return false;

    node->count = 1;
    name_len = strlen(name);
    strncpy(node->name, name, name_len);

    heap->heapsize++;
    if (heap->heapsize == 1000)
        pop(heap);
    
    int size = heap->heapsize;

    /* Added new element to the end of heap.*/
    heap->heap_array[size] = *node; 
    int curr_pos = size;

    /* Loop through till the elements have been arranged in proper order. */
    while (node->count < heap->heap_array[curr_pos/2].count) {
        heap->heap_array[curr_pos] = heap->heap_array[curr_pos/2];
        curr_pos /= 2;
    }        

    heap->heap_array[curr_pos] = *node;

    return true;
}

/* Function to increment the access count of the file with the specified 
   'name' and balance the min heap. */

void count_increment (struct heap_t *heap, char const *name) {
    int i = 1;

    for (i = 1; i <= heap->heapsize; i++) {
        if ((memcmp(heap->heap_array[i].name, name, strlen(name))) == 0)
            heap->heap_array[i].count++;
    }
}

/* Function to delete the minimum element from the min heap. */

char* pop(struct heap_t *heap) {
    int curr_pos, next = 0;
    int size = heap->heapsize;
    struct node_count last_ele = heap->heap_array[size];
    size = heap->heapsize--;
    char *name = heap->heap_array[1].name;

    for (curr_pos = 1; curr_pos*2 <= size; curr_pos = next) {
        next = curr_pos * 2;

        /* Finding the position the smallest child. It can be either (curr_pos) or (curr_pos*2)+1; */
        if (next != size && (heap->heap_array[next].count > heap->heap_array[next+1].count))
            next += 1;

        if (last_ele.count > heap->heap_array[next].count)
           heap->heap_array[curr_pos] = heap->heap_array[next];
        else 
            break;
    }

    heap->heap_array[curr_pos] = last_ele;
    return name;
}

void heap_destroy (struct heap_t *heap) {

    int i = 0;

    if (!heap)
        return;

    struct node_count *node;
    for (i = 0; i < heap->heapsize; i++) {
        node = &heap->heap_array[i];
        free(node);
    }
    free(heap);
}
