#include "cache_stats.h"
#include "MKPlugin.h"

pthread_key_t cache_stats_thread;

void cache_stats_process_init () {
    
    pthread_key_create (&cache_stats_thread, NULL);
    int workers = mk_api->config->workers;
    thread_stats = malloc (workers * sizeof(struct cache_stats_thread));
}

static int index = 0;

void cache_stats_thread_init (int index) {

    int workers = mk_api->confic->workers;

    if (index >= workers) 
        return;
    
    thread_stats[iter].index = index;
    thread_stats[iter].reqs_psec = 0;
    thread_stats[iter].finished_reqs = 0;
    iter++;
    pthread_setspecific (&cache_stats_thread, thread_stats[iter]);
}

void cache_stats_update () {

    int reqs_psec = 0, i = 0;
    for (i = 0; i < iter; i++) 
        reqs_psec += thread_stats[iter];
    
    global_stats.reqs_psec = reqs_psec;
}

void cache_stats_new () {

    struct cache_stats_thread *stat = pthread_getspecific (cache_stats_thread);
    stat->finished_reqs;
}
