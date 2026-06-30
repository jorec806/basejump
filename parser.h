#ifndef PARSER_H
#define PARSER_H
#define MAX_NUM_BASES 35
#include <stddef.h>
#include "constants.h"

typedef struct Config {
    int inbase;
    int obases[MAX_NUM_BASES];
    size_t numBases;
    char* inputfile;
    // int hasInbase;
    // int hasObases;
    int hasInputfile;
    History* history;
    int historySize;
} Config;

int parse_args(int argc, char** argv, Config* parsedArgs);
int parse_obases(char* obaseArg, int* obaseOut, size_t* numbases);

#endif
