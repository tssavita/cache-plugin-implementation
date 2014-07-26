all: monkey-cache.so
    include ../Make.common

CC       = @echo "  CC   $(_PATH)/$@"; gcc
CC_QUIET = @echo -n; gcc
CFLAGS   = -g -std=gnu99 -Wall -Wextra -g -rdynamic -I./include
LDFLAGS  = 
DEFS     = -DDEBUG -DSAFE_FREE -DMALLOC_LIBC
PROXY_OBJECTS = cache.o cache_conf.o cache_operation.o cache_stats.o hash_func.o hash_table.o min_heap.o 

-include $(PROXY_OBJECTS:.o=.d)

monkey-cache.so: $(PROXY_OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEFS) -shared -o $@ $^ -lc
