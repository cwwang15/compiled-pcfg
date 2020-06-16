//
// Created by cw on 5/12/20.
//

#include "pwdstructmap.h"
#include "hashmap.h"

map_t read_struct_map(char *filename) {
    map_t struct_map = hashmap_new();
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
        if (line_len > 0 && buf[line_len - 1] == '\n') {
            buf[line_len - 1] = '\0';
        }
        if (line_len > 1 && buf[line_len - 2] == '\r') {
            buf[line_len - 2] = '\0';
        }
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
        int error;
        pwd_struct_t *pwdStruct;
        char *key = malloc(MAX_LINE);
        snprintf(key, MAX_LINE, "%s", pair[1]);
        error = hashmap_get(struct_map, key, (void **) (&pwdStruct));
        int len = strnlen(pair[0], MAX_LINE);
        if (error == MAP_MISSING) {
            pwd_struct_t *pwdStruct1 = malloc(sizeof(pwd_struct_t));
            pwdStruct1->pwd_struct = malloc(sizeof(int) * len);
            pwd_struct_converter(key, pair[0], pwdStruct1->pwd_struct, len);
            pwdStruct1->next = pwdStruct;
            hashmap_put(struct_map, key, pwdStruct1);
        } else {
            pwd_struct_t *pwdStruct1 = malloc(sizeof(pwd_struct_t));
            pwdStruct1->pwd_struct = malloc(sizeof(int) * len);
            pwd_struct_converter(key, pair[0], pwdStruct1->pwd_struct, len);
            pwdStruct1->next = pwdStruct;
            hashmap_put(struct_map, key, pwdStruct1);
        }
    }
    fclose(fp_in);
    return struct_map;
}

int read_pwd_map(char *filename, map_t pwd_map, map_t black_list_map) {
    FILE *fp_in = fopen(filename, "r");
    if (fp_in == NULL) {
        fprintf(stderr, "failed to open file: %s", filename);
        exit(-1);
    }
    char buf[MAX_LINE];
    while (fgets(buf, MAX_LINE, fp_in) != NULL) {
        unsigned long line_len = strlen(buf);
        if (line_len > 0 && buf[line_len - 1] == '\n') {
            buf[line_len - 1] = '\0';
        }
        if (line_len > 1 && buf[line_len - 2] == '\r') {
            buf[line_len - 2] = '\0';
        }
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
        pwd_variant_t *pwdVariant;
        char *key = malloc(MAX_LINE);
        char *val = malloc(MAX_LINE);
        snprintf(key, MAX_LINE, "%s", pair[0]);
        snprintf(val, MAX_LINE, "%s", pair[1]);
        hashmap_put(black_list_map, val, val);
        int len = strnlen(key, MAX_LINE);
        hashmap_get(pwd_map, key, (void **) (&pwdVariant));
        pwd_variant_t *newPwdVariant = malloc(sizeof(pwd_variant_t));
        newPwdVariant->pwd_variant = malloc(sizeof(char) * (len + 1));
        snprintf(newPwdVariant->pwd_variant, MAX_LINE, "%s", pair[1]);
        /* if MAP_MISSING, pwdVariant will be NULL, if not, a node of pwd_variant_t */
        newPwdVariant->next = pwdVariant;
        hashmap_put(pwd_map, key, newPwdVariant);
    }
    fclose(fp_in);
    return 0;
}

int find_converter(map_t pwd_struct_map, char *reduced_struct, int **variants) {
    pwd_struct_t *pwdStruct2;
    int error = hashmap_get(pwd_struct_map, reduced_struct, (void **) (&pwdStruct2));
    if (MAP_MISSING == error) {
        return 0;
    }
    int counter = 0;
    for (pwd_struct_t *ps = pwdStruct2; ps != NULL; ps = ps->next) {
        counter++;
    }
//    variants = realloc(variants, sizeof(int *) * counter);
    int i = 0;
    for (pwd_struct_t *ps = pwdStruct2; ps != NULL; ps = ps->next, i++) {
        (variants)[i] = ps->pwd_struct;
    }
    return counter;
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
        return NOT_FOUND;
    }
    unsigned long len = strlen(source);
    for (unsigned long i = 0; i < len; i++) {
        if (source[i] == chr) {
            return i;
        }
    }
    return NOT_FOUND;
}

int pwd_struct_converter(const char *source_struct, const char *target_struct, int *pos_map, int len) {
    char n_target_struct[len + 1];
    snprintf(n_target_struct, sizeof(char) * (len + 1), "%s", target_struct);
    for (int i = 0; i < len; i++) {
        char tag = source_struct[i];
        int t_i = (int) find_char(n_target_struct, tag);
        assert(t_i >= 0 && t_i < len);
        n_target_struct[t_i] = ' ';
        pos_map[i] = t_i;
    }
    return 0;
}

int count_line(const char *filename) {
    FILE *fin = fopen(filename, "r");
    if (NULL == fin) {
        fprintf(stderr, "Failed to open %s", filename);
        exit(-1);
    }
    char buf[MAX_LINE];
    int line_count = 0;
    while (fgets(buf, MAX_LINE, fin) != NULL) {
        line_count++;
    }
    fclose(fin);
    return line_count;
}