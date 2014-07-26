#include "cache_stats.h"

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

void cache_stats_update () {
    
    int workers = mk_api->config->workers;
    int reqs_psec = 0, i = 0;
    for (i = 0; i < workers; i++) 
        reqs_psec = reqs_psec + thread_stats[iter].reqs_psec;
    
    global_stats->reqs_psec = reqs_psec;
}

void cache_stats_new () {

    struct cache_thread_stats *stat = pthread_getspecific (stats_thread);
    stat->finished_reqs += 1;
}
