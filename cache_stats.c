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

static int iter = 0;

void cache_stats_process_init () {
    
    int workers = mk_api->config->workers;
    pthread_key_create (&stats_thread, NULL);
    thread_stats = malloc (workers * sizeof(struct cache_thread_stats));
}

void cache_stats_thread_init () {

    int workers = mk_api->config->workers;
    if (iter >= workers) 
        return;

    struct cache_thread_stats *thread = &thread_stats[iter];
    
    thread->index = iter;
    thread->reqs_psec = 0;
    thread->finished_reqs = 0;
    iter++;

    pthread_setspecific (stats_thread, (void *) thread);
}

void cache_stats_update_finreqs (struct cache_thread_stats *stats) {

    time_t now;
    time(&now);
    
    int time_diff = difftime(stats->started_at, now);
    
    if (time_diff > 1000) {
        stats->started_at = now;
        stats->reqs_psec = stats->reqs_psec / (time_diff /1000);
        stats->finished_reqs = 0;
    }
}

void cache_stats_update () {
    
    int workers = mk_api->config->workers;
    int reqs_psec = 0, i = 0;
    for (i = 0; i < workers; i++) {
        cache_stats_update_finreqs(&thread_stats[i]);
        reqs_psec = reqs_psec + thread_stats[iter].reqs_psec;
    }

PLUGIN_TRACE("fin req = %d, global reqs = %d", reqs_psec, global_stats.reqs_psec);
    
    global_stats.reqs_psec = reqs_psec;
}

void cache_stats_new () {

    struct cache_thread_stats *stats = pthread_getspecific (stats_thread);
    stats->finished_reqs += 1;
    PLUGIN_TRACE("fin req = %d", stats->finished_reqs);
}
