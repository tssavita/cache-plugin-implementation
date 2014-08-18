#include <sys/timerfd.h>

#include "include/cache.h"
#include "include/stats_timer.h"
#include "include/cache_stats.h"

int stats_timer_fd;

void stats_timer_process_init() {
    struct timespec timer;
    struct itimerspec timerout;

    timer.tv_sec = 0;
    timer.tv_nsec = 1;

    timerout.it_interval = timer;
    timerout.it_value = timer;

    stats_timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd_settime(stats_timer_fd, 0, &timerout, NULL) < 0)
        perror("Setting the stats_timer has failed ! :(\n");
}

void stats_timer_thread_init() {
    int epoll_fd = mk_api->sched_worker_info()->epoll_fd;
    mk_api->epoll_add(epoll_fd, stats_timer_fd, MK_EPOLL_READ, MK_EPOLL_EDGE_TRIGGERED);
}

int stats_timer_get_fd() {
    return stats_timer_fd;
}

void stats_timer_read() { 
    char timer[8];
    int cnt = 0;
    if ((cnt = read(stats_timer_fd, timer, 8)) == 8) {
        cache_stats_update();
//        cache_file_update();
    }
}
