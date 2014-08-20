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

#ifndef _CACHE_STATS_H
#define _CACHE_STATS_H

#include <pthread.h>
#include <monkey/mk_plugin.h>
#include <sys/time.h>
#include <time.h>

struct cache_global_stats {
    int reqs_psec;
};

struct cache_thread_stats {
    int index;
    int reqs_psec;
    time_t started_at;
    int finished_reqs;
};

pthread_key_t stats_thread;

struct cache_thread_stats *thread_stats;
struct cache_global_stats global_stats;

void cache_stats_process_init ();

void cache_stats_thread_init ();

void cache_stats_update_finreqs (struct cache_thread_stats *stats);

void cache_stats_update ();

static inline void cache_stats_new () {
    struct cache_thread_stats *stats = pthread_getspecific(stats_thread);
    stats->finished_reqs += 1;
}


#endif
