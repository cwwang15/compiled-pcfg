//
// Created by cw on 5/12/20.
//

#ifndef C_HASHMAP_MASTER_PWDSTRUCTMAP_H
#define C_HASHMAP_MASTER_PWDSTRUCTMAP_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "hashmap.h"
#include "command_line.h"
#include "helper_io.h"

#define MAX_LINE (1024)

typedef struct pwd_variant_s {
    char *pwd_variant;
    struct pwd_variant_s *next;
} pwd_variant_t;


int read_pwd_map(char *filename, map_t pwd_map, map_t black_list_map);


#endif //C_HASHMAP_MASTER_PWDSTRUCTMAP_H
