#include "include/utils.h"
#include <monkey/mk_memory.h>

#include <stdio.h>
#include <string.h>

char *file_ext(char *filename) {
    int j, len;
    j = len = strlen(filename);
    
    /* looking for extension */
    while (j >= 0 && filename[j] != '.') {
        j--;
    }
    if (j <= 0) 
        return NULL;

    return (filename + j + 1);
}

