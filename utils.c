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

