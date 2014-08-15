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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "include/cache_operation.h"
#include "include/hash_table.h"
#include "include/min_heap.h"
#include "include/utils.h"
#include "MKPlugin.h"

void cache_process_init () {

    PLUGIN_TRACE ("before hash table creation");
    hash_table = table_create();
    PLUGIN_TRACE ("after hash table creation");
    heap = heap_create ();
}

void cache_thread_init () {
}

/* To check if the file is valid (whether it exists or not). */
int isvalid (struct stat *file_status) {

    mode_t mode = file_status->st_mode;

    if (  ((mode & S_IFMT) == S_IFREG) || (S_ISREG(mode))  )
        return true;
    else if (file_status->st_size <= 0)
        return false;

    return true;
}

/* To add a new file to the cache. */
struct file_t *cache_add_file (const char *path, const char *uri) {

    struct file_t *file;
    struct stat file_status;
    mode_t mode = S_IRUSR | S_IWUSR;
    PLUGIN_TRACE ("Entered add_file1 ()");
    
    /* Checking to see if the file is already present. */
    file = table_lookup (hash_table, uri);

    /* If the file was not present already in cache 
    condition becomes successful. */
    if (file == NULL) {
        if (stat(path, &file_status) == -1)
            return NULL;
    
        if (!isvalid(&file_status))
            return NULL;
    
        int fd = open (path, O_RDONLY, mode);
        if (fd == -1)
            handle_error("open");

        /* Mapping file to memory using mmap() sys call. */
        int map_length = file_status.st_size;
        void *map_content = mmap (NULL, map_length, PROT_READ, MAP_SHARED,fd, 0);
        
        if (map_content == MAP_FAILED) {
            close (fd);
            perror ("Error mapping file");
            exit(EXIT_FAILURE);
        }
        
        /* Allocating space and filling in fields of the file. */
        file = mk_api->mem_alloc(sizeof(struct file_t));
        strncpy(file->name, uri, MAX_LENGTH_NAME);
        file->content.data = map_content;
        file->content.len = map_length;
        file->count = 1;

        int htable_insert = table_insert (hash_table, uri, file);
        int mheap_insert = heap_insert (heap, uri);

        if ( htable_insert == 0 || mheap_insert == false ) {
            cache_unmap_file(file);
            return NULL;
        }
    }

    return file;
}

/* Incrementing the access count of the file each 
   time it is accessed. */
void file_access_count (struct file_t *file) {

    file->count += 1;
    count_increment(heap, file->name);
}

/* Looking up the file in the cache, with the help
   of the 'uri'*/
struct file_t *cache_lookup_file (const char *uri) {

    struct file_t *file = table_lookup (hash_table, uri);
    if (file == NULL) 
        return NULL;
    else
        file_access_count (file);

    return file;
}

/* Unmapping the file from the memory. */
void cache_unmap_file (struct file_t *file) {

    if (!file)
        return;

    close(file->fd);
    munmap (file->content.data, file->content.len);
    table_delete (hash_table, file->name);
    mk_api->mem_free(file);
}

/* Deleting the cache completely. */
void cache_destroy () {

    table_destroy (hash_table);
}
