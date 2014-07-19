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

#include "hash_func.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

/* Function to calculate the index in which the entry is entered by calculating
   the hash value.Hash function - Adding up ascii values of all characters in the 
   file name and dividing by the maximum number of lists possible in the hash table. */

int hash_func_asciisum_modulo (const char *name, size_t size) {
    unsigned int sum_ascii = 0;
    
    for (; *name; name++) {
        sum_ascii = sum_ascii + *name;
/*        while (sum_ascii > ((int) size))
            sum_ascii = sum_ascii % size;
        printf ("ascii sum = %d", sum_ascii);*/
    }

    return (sum_ascii % size);
}

int hash_func_shift4 (const char *name, size_t size) {
    int sum_shift4 = 0;

    for (; *name; name++)
        sum_shift4 = (sum_shift4 << 4) + *name;

    return (sum_shift4 % size);
}

int hash_func_mult37 (const char *name, size_t size) {
    unsigned int hash = 0;
    unsigned char *p;

    for (p = (unsigned char*) name; *p != '\0'; p++)
        hash = (37 * hash) + *p;

    printf ("Hash value = %d", hash);

    return hash % size;
}
