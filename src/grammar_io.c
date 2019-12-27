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

#include "grammar_io.h"


// Opens a file and actually loads the grammar for a particular terminal
//
// Function returns a a PcfgReplacements structure
// 
// Returns NULL if problems occur opening the file or malformed ruleset
//
PcfgReplacements* load_term_from_file(char *filename, char *type, long id) {
    
    // Pointer to the open terminal file
    FILE *fp;
    
    // Open the terminal file
    fp= fopen(filename,"r");
    
    // Check to make sure the file opened correctly
    if (fp== NULL) {
        
        //Could not open the file. Print error and return an error
        fprintf(stderr, "Error. Could not read the file: %s\n",filename);
        return NULL;
    }
    
    // Note: There will be two passes through the file
    //   1) First pass gets count info since I want to allocate memory without
    //      being wasteful, and I want to use arrays to hopefully make
    //      generating guesses faster
    //
    //   2) Actually load the terminals to the pcfgreplacement struct
    //
    
    // Holds the current line in the config file
    char buff[MAX_CONFIG_LINE];
    
    // Initalize things and get the first line of the file
    if (!fgets(buff, MAX_CONFIG_LINE , (FILE*)fp)) {
        // The file should have at least one line in it
        return NULL;
    }
    
    // This contains the number of items at the current probability
    int num_term = 1;
    
    // The previous probability we saw
    double previous_prob;
    
    // The probability of the current item;
    double prob;
    
    // A temp holder for the string value in the file
    char value[MAX_CONFIG_LINE];
    
    // Break up the first line. Using previous_prob since this will be the
    // inital value for it when comparing against other items.
    if (split_value(buff, value, &previous_prob) != 0) {
        return NULL;
    }
    
    // Initalize the first PcfgReplacement
    PcfgReplacements *terminal_pointer = (PcfgReplacements *)malloc(sizeof(struct PcfgReplacements));
    if (terminal_pointer == NULL) {
        return NULL;
    }
    terminal_pointer->parent = NULL;
    terminal_pointer->child = NULL;
    terminal_pointer->prob = previous_prob;
    terminal_pointer->id = id;
    
    // Allocate memory to store the type
    int type_len = strnlen(type,50);
    type_len += 1;
    terminal_pointer->type = (char *)malloc(type_len * sizeof(char *));
    if (terminal_pointer->type == NULL) {
        return NULL;
    }
    strncpy(terminal_pointer->type, type, type_len);
    
    PcfgReplacements *cur_pointer = terminal_pointer;
        
    // Loop through the config file
    while (fgets(buff, MAX_CONFIG_LINE , (FILE*)fp)) {
        
        if (split_value(buff, value, &prob) != 0) {
            return NULL;
        }
        
        if (prob == previous_prob) {
            num_term++;
        }
        else {
            // Finalize this structure
            cur_pointer->size = num_term;
            if ((cur_pointer->value = (char **) malloc(num_term * sizeof(char *))) == NULL) {
                return NULL;
            }

            // intialize the next, and make it current
            previous_prob = prob;
            num_term = 1;
            
            cur_pointer->child = (PcfgReplacements *)malloc(sizeof(struct PcfgReplacements));
            if (cur_pointer->child == NULL) {
                return NULL;
            }
            
            cur_pointer->child->parent = cur_pointer;
            cur_pointer = cur_pointer->child;
            cur_pointer->child = NULL;
            cur_pointer->prob = prob;
            cur_pointer->id = id;
    
            // Allocate memory to store the type
            cur_pointer->type = (char *)malloc(type_len * sizeof(char *));
            if (cur_pointer->type == NULL) {
                return NULL;
            }
            strncpy(cur_pointer->type, type, type_len);
        }
    }
    
    //Save the size of the last items
    cur_pointer->size = num_term;
    if ((cur_pointer->value = (char **) malloc(num_term * sizeof(char *))) == NULL) {
        return NULL;
    }
    
    // Now on to the second loop where we will save all of the items
    
    // Reset the file pointer to the start of the file
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fprintf(stderr, "Error. Could not seek in: %s\n",filename);
        return NULL;
    }
    cur_pointer = terminal_pointer;
    num_term = 0;
    
    while (fgets(buff, MAX_CONFIG_LINE , (FILE*)fp)) {
        
        // Advance to the next container if needed
        if (num_term == cur_pointer->size) {
            cur_pointer = cur_pointer->child;
            num_term = 0;
        }
        
        if (split_value(buff, value, &prob) != 0) {
            fprintf(stderr, "Error. Could not split value in rules file\n");
            return NULL;
        }
        // Sanity checking
        if (prob != cur_pointer->prob) {
            fprintf(stderr, "Error. Probability mismatch in rules file\n");
            return NULL;
        }
        
        // Save the actual value
        int value_len = strnlen(value, MAX_CONFIG_LINE);

        cur_pointer->value[num_term] = malloc((value_len +1) * sizeof(char *));
        if (cur_pointer->value[num_term] == NULL) {
            return NULL;
        }
        strncpy(cur_pointer->value[num_term], value, value_len +1);
        
        num_term++;
        
    }
    
    return terminal_pointer;
}


// Loads the grammar for a particular terminal
//
// Function returns 0 if it worked ok
//
// If an error occurs function returns 1
//
int load_terminal(char *config_filename, char *base_directory, char *structure, char *type, PcfgReplacements *grammar_item[]) {
    
    // Get the folder where the files will be saved
    char section_folder[MAX_CONFIG_LINE];
    
    if (get_key(config_filename, structure, "directory", section_folder) != 0) {
        fprintf(stderr, "Could not get folder name for section. Exiting\n");
        return 1;
    }
    
    // Get the filenames associated with the structure
    char result[256][MAX_CONFIG_ITEM];
    int list_size;
    if (config_get_list(config_filename, structure, "filenames", result, &list_size, 256) != 0) {
        fprintf(stderr, "Error reading the config for a rules file. Exiting\n");
        return 1;
	}

    //Load each of the files
    for (int i = 0; i< list_size; i++) {

        //Find the id for this file, aka A1 vs A23. This is the 1, or 23
        char *end_pos = strchr(result[i],'.');
        
        //If there isn't a .txt, this is an invalid file
        if (end_pos == NULL) {
            return 1;
        }
        
        long id = strtol(result[i],&end_pos, 10);
        
        // Check to make sure it was a number
        if ((errno == EINVAL) || (errno == ERANGE))
        {
            fprintf(stderr, "Invalid File name found in rules. Exiting\n");
            return 1;
        }
         
        // Make sure the id falls within the acceptable range
        if (id <= 0) {
            fprintf(stderr, "Invalid File name found in rules. Exiting\n");
            return 1;
        }      

        // Make sure the value isn't too long for the current compiled pcfg_guesser
        if (id > MAX_TERM_LENGTH) {
            continue;
        }
        
        char filename[PATH_MAX];
        snprintf(filename, PATH_MAX, "%s%s%c%s", base_directory,section_folder,SLASH,result[i]);
        
        grammar_item[id] = load_term_from_file(filename, type, id);
        
        if (grammar_item[id] == NULL) {
            return 1;
        }

    }

    return 0;
}


// Loads a ruleset/grammar from disk
//
// Function returns a non-zero value if an error occurs
//     1 = problem opening the file
//     2 = malformed ruleset
//     3 = unsupported feature/ruleset
//
int load_grammar(char *arg_exec, struct program_info program_info, PcfgGrammar *pcfg) {
    
    // Directory the executable is running in
    // Adding in a plus 1 to PATH_MAX to deal with a potential edge case
    // when formatting this to represent the directory. Should never happen
    // but still...
    char exec_directory[PATH_MAX + 1];
    strncpy(exec_directory, arg_exec, PATH_MAX);
    
    // Hate doing it this way but haven't found a good way to get the directory
    char *tail_slash = strrchr(exec_directory, SLASH);
    
    // Holds the return value of function calls
    int ret_value;
    
    // Using the current directory
    if (tail_slash == NULL) {
        snprintf(exec_directory, PATH_MAX, ".%c", SLASH);
    }
    // Using a directory passed in via argv[0]
    else {   
        tail_slash[1]= '\0';
    }
    
    // Create the base directory to load the rules from
    char base_directory[FILENAME_MAX];
    snprintf(base_directory, FILENAME_MAX, "%s%c", program_info.rule_name, SLASH);
    
    fprintf(stderr, "Loading Ruleset:%s\n",base_directory);
    
    // Save the config file name for easy reference
    char config_filename[FILENAME_MAX];
    snprintf(config_filename, FILENAME_MAX, "%sconfig.ini", base_directory);
    
    // Currently we only support UTF-8 for the compiled version, so ensure
    // the ruleset uses that
    //
    // Note: The Python version of the pcfg_guesser supports other encoding
    //       types.
    ret_value = check_encoding(config_filename);
    if (ret_value != 0) {
        return ret_value;
    }
    
    // Read in the alpha terminals
    if (load_terminal(config_filename, base_directory, "BASE_A", "A", pcfg->alpha) != 0) {
        fprintf(stderr, "Error reading the rules file. Exiting\n");
        return 1;
	}

    // Read in the capitalization masks
    if (load_terminal(config_filename, base_directory, "CAPITALIZATION", "C", pcfg->capitalization) != 0) {
        fprintf(stderr, "Error reading the rules file. Exiting\n");
        return 1;
	}
    // Read in the digit terminals 
    if (load_terminal(config_filename, base_directory, "BASE_D", "D", pcfg->digits) != 0) {
        fprintf(stderr, "Error reading the rules file. Exiting\n");
        return 1;
	}
    
    // Read in the years terminals 
    if (load_terminal(config_filename, base_directory, "BASE_Y", "Y", pcfg->years) != 0) {
        fprintf(stderr, "Error reading the rules file. Exiting\n");
        return 1;
	}
    // Read in the "other" terminals 
    if (load_terminal(config_filename, base_directory, "BASE_O", "O", pcfg->other) != 0) {
        fprintf(stderr, "Error reading the rules file. Exiting\n");
        return 1;
	}
    
    // Read in the conteXt sensitive terminals
    if (load_terminal(config_filename, base_directory, "BASE_X", "X", pcfg->x) != 0) {
        fprintf(stderr, "Error reading the rules file. Exiting\n");
        return 1;
	}
    
    // Read in the keyboard combo terminals
    if (load_terminal(config_filename, base_directory, "BASE_K", "K", pcfg->keyboard) != 0) {
        fprintf(stderr, "Error reading the rules file. Exiting\n");
        return 1;
	}
    
    // Now read in the base structures. Note, this doesn't need to be done last
    // but depending on what enhancements are done in the future it's good
    // practice to process these at the end.
    if (load_base_structures(config_filename, base_directory, &pcfg->base_structures) != 0) {
        fprintf(stderr, "Error reading the base_structure file in the rules. Exiting\n");
        return 1;
	}
    
    return 0;
}
