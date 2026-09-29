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

    int hasInputfile;
    History* history;
    int historySize;
} Config;

typedef struct Token {
    const char* value;
    size_t size;
} Token;

int parse_args(int argc, char** argv, Config* parsedArgs);
int parse_obases(char* obaseArg, int* obaseOut, size_t* numbases);
int tokenizer(const char* str, const char* separators, Token** result, size_t* len);

int evaluate_expression(const char* expr, unsigned long long* result);
char* convert_any_base_to_base_ten(const char* input, int base);
char* convert_int_to_str_any_base(const char* input, int base);
char* convert_expression(const char* expr, int inputBase, int outputBase);
unsigned long long convert_str_to_any_base(const char* input, int base);

#endif
