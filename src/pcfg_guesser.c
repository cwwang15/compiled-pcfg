//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Pretty Cool Fuzzy Guesser (PCFG)
//  --Probabilistic Context Free Grammar (PCFG) Password Guessing Program
//
//  Written by Matt Weir
//  Special thanks to the John the Ripper and Hashcat communities where some 
//  of the code was copied from. And thank you whoever is reading this. Be good!
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//

#include "pcfg_guesser.h"

long long guess_number;
long long cur_gen_num;
char *guesses_file;
FILE *foutp;
map_t pwdStructMap;
map_t pwdVariantMap;
// these should be delete
#include <sys/time.h>

struct timeval start, end;

void recursive_guess(PQItem *pq_item, int base_pos, char *cur_guess, int start_point) {

    int new_start = start_point;

    for (int i = 0; i < pq_item->pt[base_pos]->size; i++) {

        // This is a capitalization section

        if (strncmp(pq_item->pt[base_pos]->type, "C", 2) == 0) {
            // Go backward to the previous section and apply the mask
            // Note, if someone messed with the ruleset this could cause issues, so
            // need to do some sanity checking on the bounds
            int mask_len = strnlen(pq_item->pt[base_pos]->value[i], MAX_TERM_LENGTH + 1);
            //Sanity check on length
            if (mask_len > start_point) {
                fprintf(stderr, "Error with the capitalization masks\n");
                return;
            }

            // I'm pretty sure this isn't going to be sufficient for UTF-8
            // characters, but that's a rabbit hole I'm going to have to dive
            // into at a later point.
            for (int y = 0; y < mask_len; y++) {
                //lowercase the letter
                if (pq_item->pt[base_pos]->value[i][y] == 'L') {
                    cur_guess[start_point - mask_len + y] = tolower(cur_guess[start_point - mask_len + y]);
                } else {
                    cur_guess[start_point - mask_len + y] = toupper(cur_guess[start_point - mask_len + y]);
                }
            }

        } else {
            strncpy(cur_guess + start_point, pq_item->pt[base_pos]->value[i], MAX_GUESS_SIZE - start_point);
            new_start = strnlen(cur_guess, MAX_GUESS_SIZE);
        }

        // If this is the last item, generate a guess
        if (base_pos == (pq_item->size - 1)) {
            cur_gen_num++;
//            fprintf(stderr, "%llu: ", cur_gen_num);
            fputs(cur_guess, foutp);
            fputs("\n", foutp);
            char *cur_guess_struct = pwd_struct_extractor(cur_guess);
//            fprintf(stderr, "0, ");
            int **variants = malloc(sizeof(int *) * 100);
//            fprintf(stderr, "1, ");
            int variants_count = find_converter(pwdStructMap, cur_guess_struct, variants);
//            fprintf(stderr, "2, ");
            int pwd_len = strnlen(cur_guess, MAX_LINE);
            for (int v = 0; v < variants_count; v++) {
//                fprintf(stderr, "3, ");
                char variant[pwd_len + 1];
                variant[pwd_len] = '\0';
                int *pos_map = variants[v];
                for (int p = 0; p < pwd_len; p++) {
                    variant[pos_map[p]] = cur_guess[p];
                }
                fputs(variant, foutp);
                fputs("\n", foutp);
                cur_gen_num++;
                if (cur_gen_num >= guess_number) {
                    gettimeofday(&end, NULL);
                    double timeuse =
                            (end.tv_sec - start.tv_sec) + ((double) (end.tv_usec - start.tv_usec) / 1000000);
                    printf("timeuse: %fs\n", timeuse);
                    printf("Done! The speed is %f\n", guess_number / timeuse);
                    exit(0);
                }
            }
//            fprintf(stderr, "4, ");

            free(variants);
            free(cur_guess_struct);
            variants = NULL;
            cur_guess_struct = NULL;
            if (cur_gen_num >= guess_number) {
                gettimeofday(&end, NULL);
                double timeuse = (end.tv_sec - start.tv_sec) + ((double) (end.tv_usec - start.tv_usec) / 1000000);
                printf("timeuse: %fs\n", timeuse);
                printf("Done! The speed is %f\n", guess_number / timeuse);
                exit(0);
            }
//            fprintf(stderr, "ok\n");
            // printf("guess: %s\n",cur_guess);
        }
            // Not the last item so doing this recursivly
        else {
            recursive_guess(pq_item, base_pos + 1, cur_guess, new_start);
        }
    }
}


// Generates guesses from a parse_tree
void generate_guesses(PQItem *pq_item) {

    //Used for debugging
    //int i;
    //for (i=0; i < pq_item->size; i++) {
    //    printf("%s%li ",pq_item->pt[i]->type, pq_item->pt[i]->id);
    //}
    //printf("\n");

    char guess[MAX_GUESS_SIZE];
    recursive_guess(pq_item, 0, guess, 0);

}


// The main program
int main(int argc, char *argv[]) {

    // Holds the info from the command line
    struct program_info program_info;

    // Parse the command line
    if (parse_command_line(argc, argv, &program_info) != 0) {
        fprintf(stderr, "Error, parsing command line. Exiting\n");
        return 1;
    }
    guess_number = program_info.guess_number;
    cur_gen_num = 0;
    guesses_file = program_info.guesses_file;
    // get path of grammar and pwd_struct_map
    char *grammar_path = malloc(PATH_MAX);
    snprintf(grammar_path, PATH_MAX,
             "%s%s", program_info.rule_name, "/Grammar/grammar.txt");
    char *struct_map_path = malloc(PATH_MAX);
    snprintf(struct_map_path, PATH_MAX, "%s%s", program_info.rule_name, "/Grammar/structmap.txt");
    char *pwd_map_path = malloc(PATH_MAX);
    snprintf(pwd_map_path, PATH_MAX, "%s%s", program_info.rule_name, "/Grammar/pwdmap.txt");
    printf("grammar path: %s\nstruct map path: %s\n", grammar_path, struct_map_path);
    pwdStructMap = read_struct_map(struct_map_path);
    pwdVariantMap = read_pwd_map(pwd_map_path);
    pwd_variant_t *pwdVariant;
    int myerro = hashmap_get(pwdVariantMap, "321321..", (void **) (&pwdVariant));
    printf("hello, errno: %d\n", myerro);
    if (myerro != MAP_MISSING) {
        printf("%s\n", pwdVariant->pwd_variant);
    }
    free(grammar_path);
    free(struct_map_path);
    free(pwd_map_path);
    // Print the startup banner
    print_banner(program_info.version);

    // Create the empty grammar
    PcfgGrammar pcfg;


    // Intiazlie the grammar and Priority Queue
    if (load_grammar(argv[0], program_info, &pcfg) != 0) {
        fprintf(stderr, "Error loading ruleset. Exiting\n");
        return 1;
    }
    foutp = fopen(guesses_file, "w");
    if (NULL == foutp) {
        fprintf(stderr, "error open file %s\n", guesses_file);
        return 1;
    }
    fprintf(stderr, "Initailizing the Priority Queue\n");
    priority_queue_t *pq;

    initialize_pcfg_pqueue(&pq, &pcfg);

    fprintf(stderr, "Starting to generate guesses\n");
    gettimeofday(&start, NULL);
    // Start generating guesses
    while (!priority_queue_empty(pq)) {
        PQItem *pq_item = pcfg_pq_pop(pq);
        if (pq_item == NULL) {
            printf("Memory allocation error when popping item from pqueue\n");
            return 1;
        }

        generate_guesses(pq_item);

        free(pq_item->pt);
        free(pq_item);
    }


    return 0;
}
