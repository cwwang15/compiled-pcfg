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
#define NOT_FOUND (-1)

typedef struct pwd_struct_s {
    int *pwd_struct;
    struct pwd_struct_s *next;
} pwd_struct_t;

typedef struct pwd_variant_s {
    char *pwd_variant;
    struct pwd_variant_s *next;
} pwd_variant_t;

//map_t read_struct_map(char *filename);

int read_pwd_map(char *filename, map_t pwd_map, map_t black_list_map);

//int find_converter(map_t pwd_struct_map, char *reduced_struct, int **variants);

//char *pwd_struct_extractor(const char *pwd);

unsigned long find_char(char source[], char chr);

int pwd_struct_converter(const char *pwd, const char *target_struct, int *pos_map, int len);

//int count_line(const char *filename);

int load_terminals2map(struct program_info program_info, map_t terminals_map);

//int load_terminals2map(char *grammar_path, map_t terminals_map);
int sparsity(const char *pwd, FILE *fp, map_t terminals_map);

#endif //C_HASHMAP_MASTER_PWDSTRUCTMAP_H
