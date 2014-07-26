#ifndef _CACHE_STATS_H
#define _CACHE_STATS_H

struct cache_global_stats {
    int reqs_psec;
};

struct cache_thread_stats {
    int index;
    int reqs_psec;
    int finished_reqs;
};

int workers;
struct cache_thread_stats *thread_stats;
struct cache_global_stats *global_stats;
int iter = 0;

void cache_stats_process_init ();

void cache_stats_thread_init ();

void cache_stats_update ();

void cache_stats_new ();

#endif
