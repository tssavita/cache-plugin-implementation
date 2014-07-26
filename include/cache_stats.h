#ifndef _CACHE_STATS_H
#define _CACHE_STATS_H

#include <pthread.h>
#include "MKPlugin.h"

struct cache_global_stats {
    int reqs_psec;
};

struct cache_thread_stats {
    int index;
    int reqs_psec;
    int finished_reqs;
};

pthread_key_t stats_thread;
struct cache_thread_stats *thread_stats;
struct cache_global_stats *global_stats;

void cache_stats_process_init ();

void cache_stats_thread_init ();

void cache_stats_update ();

void cache_stats_new ();

#endif
