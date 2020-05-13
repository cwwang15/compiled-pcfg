//
// Created by cw on 5/12/20.
//

#include "pwdstructmap.h"
#include "hashmap.h"

map_t read_pwd_struct_map(char *filename) {
    map_t pwd_struct_map = hashmap_new();
    FILE *fp_in = fopen(filename, "r");
    if (fp_in == NULL) {
        perror("failed to open file");
        exit(-1);
    }
    char buf[MAX_LINE];
    unsigned long line_len;
    int counter = 0;
    while (fgets(buf, MAX_LINE, fp_in) != NULL) {
        counter += 1;
        line_len = strlen(buf);
        buf[line_len - 1] = '\0';
        char *token;
        int i;
        char delim[] = "\t";
        char *pair[2];
        for (token = strtok(buf, delim), i = 0; token != NULL; token = strtok(NULL, delim), i += 1) {
            if (i >= 2) {
                perror("Unexpected index: ");
                exit(-1);
            }
            pair[i] = token;
        }
//        char *origin_pwd_struct = pair[0];
//        char reduce_pwd_struct[KEY_MAX_LENGTH] = pair[1];
        int error;
        pwd_struct_t *pwdStruct;
        char *key = malloc(MAX_LINE);
        snprintf(key, MAX_LINE, "%s", pair[1]);
        error = hashmap_get(pwd_struct_map, key, (void **) (&pwdStruct));
        if (error == MAP_MISSING) {
            pwd_struct_t *pwdStruct1 = malloc(sizeof(pwd_struct_t));
            pwdStruct1->pwd_struct = malloc(sizeof(char) * (strlen(pair[0]) + 1));
            strncpy(pwdStruct1->pwd_struct, pair[0], strlen(pair[0]));
            pwdStruct1->next = pwdStruct;
            hashmap_put(pwd_struct_map, key, pwdStruct1);
        } else {
//            printf("key: %s\n", pair[1]);
//            for (pwd_struct_t *ps = pwdStruct; ps != NULL; ps = ps->next) {
//                printf("%s\n", ps->pwd_struct);
//            }
//            printf("-------------------------\n");
            pwd_struct_t *pwdStruct1 = malloc(sizeof(pwd_struct_t));
            pwdStruct1->pwd_struct = malloc(sizeof(char) * (strlen(pair[0]) + 1));
            strncpy(pwdStruct1->pwd_struct, pair[0], strlen(pair[0]));
            pwdStruct1->next = pwdStruct;
            hashmap_put(pwd_struct_map, key, pwdStruct1);
        }
    }
    return pwd_struct_map;
}

origin_structs_t *find_converter(map_t pwd_struct_map, char *reduced_struct) {
    pwd_struct_t *pwdStruct2;
    int error = hashmap_get(pwd_struct_map, reduced_struct, (void **) (&pwdStruct2));
    origin_structs_t *originStructs = malloc(sizeof(origin_structs_t));
    if (MAP_MISSING == error) {
        originStructs->len = 0;
        originStructs->variants = malloc(4);
        return originStructs;
    }
    int counter = 0;
    for (pwd_struct_t *ps = pwdStruct2; ps != NULL; ps = ps->next) {
        counter += 1;
    }
    originStructs->len = counter;
    originStructs->variants = malloc(sizeof(char *) * counter);
    int i = 0;
    for (pwd_struct_t *ps = pwdStruct2; ps != NULL; ps = ps->next, i += 1) {
        originStructs->variants[i] = malloc(sizeof(char) * (strlen(reduced_struct) + 1));
        snprintf(originStructs->variants[i], strlen(reduced_struct) + 1, "%s", ps->pwd_struct);
    }
    return originStructs;
}

char *pwd_struct_extractor(const char *pwd) {
    unsigned long len = strlen(pwd);
    char *pwd_struct = malloc(sizeof(char) * (len + 1));
    pwd_struct[len] = '\0';
    for (unsigned long i = 0; i < len; i++) {
        if ('a' <= pwd[i] && pwd[i] <= 'z') {
            pwd_struct[i] = 'L';
        } else if ('A' <= pwd[i] && pwd[i] <= 'Z') {
            pwd_struct[i] = 'U';
        } else if ('0' <= pwd[i] && pwd[i] <= '9') {
            pwd_struct[i] = 'D';
        } else {
            pwd_struct[i] = 'S';
        }
    }
    return pwd_struct;
}

unsigned long find_char(char source[], char chr) {

    if (source == NULL) {
        return -1;
    }
    unsigned long len = strlen(source);
    for (unsigned long i = 0; i < len; i++) {
        if (source[i] == chr) {
            return i;
        }
    }
    return -1;
}

char *pwd_struct_converter(const char *pwd, const char *target_struct) {
    unsigned long len = strlen(pwd);
    if (len != strlen(target_struct)) {
        perror("unmatched length of pwd and struct");
        exit(-1);
    }
    char n_pwd[len + 1];
    n_pwd[len] = '\0';
    char *source_struct = pwd_struct_extractor(pwd);
    char n_target_struct[len + 1];
    snprintf(n_target_struct, sizeof(char) * (len + 1), "%s", target_struct);
    for (unsigned long i = 0; i < len; i++) {
        char tag = source_struct[i];
        unsigned long t_i = find_char(n_target_struct, tag);
        n_target_struct[t_i] = ' ';
        n_pwd[t_i] = pwd[i];
    }

    char *ret = malloc(sizeof(char) * (len + 1));
    snprintf(ret, sizeof(char) * (len + 1), "%s", n_pwd);
    return ret;
}