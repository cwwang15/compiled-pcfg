//
// Created by cw on 5/12/20.
//

#include "pwdstructmap.h"
#include "hashmap.h"

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

