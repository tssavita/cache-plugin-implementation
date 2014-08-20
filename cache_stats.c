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

#include <monkey/mk_api.h>
#include "include/cache_stats.h"
#include <time.h>

static int workers = 0;

pthread_key_t stats_thread;

int iter = 0;

void cache_stats_process_init () {
    pthread_key_create(&stats_thread, NULL);
    iter = mk_api->config->workers;
    thread_stats = malloc (iter * sizeof(struct cache_thread_stats));
}

void cache_stats_thread_init () {

    if (workers >= iter) 
        return;

    struct cache_thread_stats *thread = &thread_stats[workers];
    
    thread->index = workers;
    thread->reqs_psec = 0;
    thread->finished_reqs = 0;

    time_t stats_now;
    time(&stats_now);
    thread->started_at = stats_now;
    
    pthread_setspecific (stats_thread, (void *) thread);
    workers++;
}

void cache_stats_update_finreqs (struct cache_thread_stats *stats) {

    time_t now;
    time(&now);
    
    int time_diff = difftime(now,stats->started_at);
    PLUGIN_TRACE("time_diff = %d", time_diff);

    if (time_diff > 60) {

        PLUGIN_TRACE("stats->finished_reqs = %d, stats->reqs_psec = %d", stats->finished_reqs, stats->reqs_psec);
        stats->started_at = now;
        stats->reqs_psec = stats->finished_reqs; // / (time_diff /10.0);
        stats->finished_reqs = 0;
    }
}

void cache_stats_update () {
    
    int reqs_psec = 0, i = 0;
    for (i = 0; i < iter; i++) {
        cache_stats_update_finreqs(&thread_stats[i]);
        reqs_psec += thread_stats[i].reqs_psec;
    }

    global_stats.reqs_psec = reqs_psec;
}

void cache_stats_new () {
    struct cache_thread_stats *stats = malloc(sizeof(struct cache_thread_stats));
    memset(stats, 0, sizeof(struct cache_thread_stats));
    stats = pthread_getspecific(stats_thread);

    stats->finished_reqs += 1;
}
