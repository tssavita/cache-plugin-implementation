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

#include "include/cache.h"
#include "include/hash_table.h"
#include "include/hash_func.h"
#include "include/utils.h"
#include "include/cJSON.h"
#include "include/cache_operation.h"
#include "include/cache_stats.h"
#include "include/cache_conf.h"

#include <monkey/mk_plugin.h>
#include <monkey/mk_memory.h>
#include <monkey/mk_api.h>

/* Create hash table structure. */
struct table_t *table_create () {

    struct table_t *table = mk_api->mem_alloc(sizeof(struct table_t));

    if (!table)
        return NULL;

    table->table_size = TABLE_SIZE;
    size_t space = TABLE_SIZE * (sizeof(struct node_t));
    table->table_list = mk_api->mem_alloc_z(space);
    
    return table;
}

/* Insert key into the table.  */
int table_insert (struct table_t *table, const char *key, void *data) {

    int index = hash_func_asciisum_modulo(key, table->table_size);
    PLUGIN_TRACE ("Finding out hash of the file name requested = %d\n", index);

    struct node_t *node = mk_api->mem_alloc_z(sizeof(struct node_t)); 

    memcpy(node->key, key, MAX_PATH_LEN);
    node->data = data;

    node->next = table->table_list[index];
    table->table_list[index] = node;

    return true;
}

void *table_file_info(struct table_t *table, void *result) {
    struct node_t *temp = mk_api->mem_alloc(sizeof(struct node_t));

    int i = 0;
    for (i = 0; i < table->table_size; i++) {
        for (temp = table->table_list[i]; temp; temp = temp->next) {
            result = cJSON_stats_file(temp->key, temp->data, result);
        }
    }
    return result;
}

/* Looking up a value in the hash table. */
void *table_lookup (struct table_t *table, const char *key) {

    int index = hash_func_asciisum_modulo(key, table->table_size);
    struct node_t *temp = mk_api->mem_alloc_z(sizeof(struct node_t));
    temp = table->table_list[index];

    if (!temp) 
        return NULL;

    for (temp = table->table_list[index]; temp; temp = temp->next) {
        if (temp->key == key || (memcmp(temp->key, key, strlen(key)) ==0)) {
            PLUGIN_TRACE ("Found file\n");
            return temp->data;
        }
    }

    PLUGIN_TRACE ("Not found\n");
    return NULL;
}

/* Deleting an element from the table. */
void *table_delete (struct table_t *table, const char *key) {

    int index = hash_func_asciisum_modulo(key, table->table_size);
    struct node_t *temp1 = table->table_list[index], *temp2;
    void *data = NULL;

    if (!temp1) 
        return NULL;

    if (temp1->key == key || (memcmp(temp1->key, key, strlen(key))==0)) {
        table->table_list[index] = temp1->next;
        data = temp1->next;
        free(temp1);
    }
    else {
        for (temp1 = table->table_list[index], temp2 = temp1; temp1 != NULL; temp2 = temp1, temp1 = temp1->next) {
            if (temp1->key == key || (memcmp (temp1->key, key, strlen(key))==0)) {
                temp2->next = temp1->next;
                data = temp1->data;
                free(temp1);
                break;
            }
        }
    }
    
    return data;
}

/* Deleting the table including the structure 
   and freeing the space. */
void table_destroy (struct table_t *table) {
    struct node_t *temp1;
    int i;

    for (i = 0; i < table->table_size; i++) {
        struct node_t *temp2 = temp1;
        for (temp1 = table->table_list[i]; temp1 != NULL; temp1 = temp2) {
            temp2 = temp1->next;
            free(temp1);
        }   
    }
    PLUGIN_TRACE("Destroy");
    free (table->table_list);
    free (table);
    PLUGIN_TRACE("Destroy");
}
