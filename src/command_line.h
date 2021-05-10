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


#ifndef _COMMAND_LINE_H
#define _COMMAND_LINE_H


#include <argp.h>
#include "global_def.h"
#include <stdlib.h>

// Contains results of parsing the command line
struct program_info {
    int debug;                // The -d flag
    char *rule_name;          // The rule name, -r
    char *version;
    char *min_supported_version; // The oldedst supported ruleset
    long long guess_number;
};


// Parses the command line
extern int parse_command_line(int argc, char **argv, struct program_info *program_info);

#endif
