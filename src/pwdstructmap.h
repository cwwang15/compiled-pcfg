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

#define MAX_LINE (1024)
#define NOT_FOUND -1
typedef struct pwd_struct_s {
    int *pwd_struct;
    struct pwd_struct_s *next;
} pwd_struct_t;

typedef struct pwd_variant_s {
    char *pwd_variant;
    struct pwd_variant_s *next;
} pwd_variant_t;

typedef struct not_cracked_s {
    char *not_cracked;
    struct not_cracked_s *next;
} not_cracked_t;

map_t read_struct_map(char *filename);

map_t read_pwd_map(char *filename);
map_t read_not_cracked(char *filename);
int find_converter(map_t pwd_struct_map, char *reduced_struct, int **variants);

char *pwd_struct_extractor(const char *pwd);

unsigned long find_char(char source[], char chr);

int pwd_struct_converter(const char *pwd, const char *target_struct, int *pos_map, int len);

int count_line(const char *filename);

#endif //C_HASHMAP_MASTER_PWDSTRUCTMAP_H
