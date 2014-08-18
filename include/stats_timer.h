#ifndef _STATS_TIMER_H_
#define _STATS_TIMER_H_

void stats_timer_process_init();
void stats_timer_thread_init();
int stats_timer_get_fd();
void stats_timer_read();

#endif
