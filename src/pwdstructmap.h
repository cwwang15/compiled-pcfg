//
// Created by cw on 5/12/20.
//

#ifndef C_HASHMAP_MASTER_PWDSTRUCTMAP_H
#define C_HASHMAP_MASTER_PWDSTRUCTMAP_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hashmap.h"

#define MAX_LINE (1024)
typedef struct pwd_struct_s {
    char *pwd_struct;
    struct pwd_struct_s *next;
} pwd_struct_t;
typedef struct origin_structs_s {
    char **variants;
    int len;
} origin_structs_t;

map_t read_pwd_struct_map(char *filename);

origin_structs_t *find_converter(map_t pwd_struct_map, char *reduced_struct);

char *pwd_struct_extractor(const char *pwd);

unsigned long find_char(char source[], char chr);
char *pwd_struct_converter(const char *pwd, const char *target_struct);
#endif //C_HASHMAP_MASTER_PWDSTRUCTMAP_H
