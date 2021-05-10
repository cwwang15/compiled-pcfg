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
// these should be delete

struct timeval start, end;

void recursive_guess(PQItem *pq_item, int base_pos, char *cur_guess, int start_point) {

    int new_start = start_point;

    for (int i = 0; i < pq_item->pt[base_pos]->size; i++) {

        // This is a capitalization section

        if (strncmp(pq_item->pt[base_pos]->type, "C", 1) == 0) {
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
                int idx = start_point - mask_len + y;
                char c = cur_guess[idx];
                char mask = pq_item->pt[base_pos]->value[i][y];
                if (mask == 'U' && 'a' <= c && c <= 'z') {
                    cur_guess[idx] = (char) toupper((int) c);
                } else if (mask == 'L' && 'A' <= c && c <= 'Z') {
                    cur_guess[idx] = (char) tolower((int) c);
                }
//                if (pq_item->pt[base_pos]->value[i][y] == 'L') {
//                    cur_guess[start_point - mask_len + y] = (char) tolower(cur_guess[start_point - mask_len + y]);
//                } else {
//                    cur_guess[start_point - mask_len + y] = (char) toupper(cur_guess[start_point - mask_len + y]);
//                }
            }

        } else {
            strncpy(cur_guess + start_point, pq_item->pt[base_pos]->value[i], MAX_GUESS_SIZE - start_point);
            new_start = strnlen(cur_guess, MAX_GUESS_SIZE);
        }

        // If this is the last item, generate a guess
        if (base_pos == (pq_item->size - 1)) {
            cur_gen_num++;
            fputs(cur_guess, stdout);
            fputc('\n', stdout);
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
    // get path of grammar and pwd_struct_map
    char *pwd_map_path = malloc(PATH_MAX);
    snprintf(pwd_map_path, PATH_MAX, "%s%c%s%c%s", program_info.rule_name, SLASH, "Mixing", SLASH, "all.txt");

    // Print the startup banner
    print_banner(program_info.version);

    // Create the empty grammar
    PcfgGrammar pcfg;


    // Intiazlie the grammar and Priority Queue
    if (load_grammar(argv[0], program_info, &pcfg) != 0) {
        fprintf(stderr, "Error loading ruleset. Exiting\n");
        return 1;
    }
    FILE *fp_in = fopen(pwd_map_path, "r");
    if (fp_in == NULL) {
        fprintf(stderr, "failed to open file: %s", pwd_map_path);
        free(pwd_map_path);
        return 1;
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
        fprintf(stdout, "%s\n", pair[1]);
        cur_gen_num++;
    }
    fclose(fp_in);
    fprintf(stderr, "Initializing the Priority Queue\n");
    priority_queue_t *pq;

    initialize_pcfg_pqueue(&pq, &pcfg);

    fprintf(stderr, "Starting to generate guesses\n");
    gettimeofday(&start, NULL);
    // Start generating guesses
    while (!priority_queue_empty(pq)) {
        PQItem *pq_item = pcfg_pq_pop(pq);
        if (pq_item == NULL) {
            fprintf(stderr, "Memory allocation error when popping item from pqueue\n");
            return 1;
        }

        generate_guesses(pq_item);
        gettimeofday(&end, NULL);
        double timeuse = (double) (end.tv_sec - start.tv_sec) + ((double) (end.tv_usec - start.tv_usec) / 1000000);
        if (cur_gen_num > guess_number || timeuse > 2.0) {
            exit(0);
        }
        free(pq_item->pt);
        free(pq_item);
    }
    return 0;
}
