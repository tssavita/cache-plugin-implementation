all: monkey-cache.so
    include ../Make.common

CC       = @echo "  CC   $(_PATH)/$@"; gcc
CC_QUIET = @echo -n; gcc
CFLAGS   =   -std=gnu99 -Wall -Wextra -g -rdynamic
LDFLAGS  = 
DEFS     = -DDEBUG  -DMALLOC_LIBC
PROXY_OBJECTS = cache.o cache_conf.o cache_operation.o cache_stats.o stats_timer.o hash_func.o hash_table.o min_heap.o cJSON.o utils.o

-include $(PROXY_OBJECTS:.o=.d)

monkey-cache.so: $(PROXY_OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEFS) -shared -o $@ $^ -lc
