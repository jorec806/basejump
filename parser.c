#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"
#include "constants.h"


int parse_args(int argc, char** argv, Config* parsedArgs);
int parse_obases(char* obaseArg, int* obaseOut, size_t* numbases);

static int args_constraints(int argc, char** argv, Config* cfg);
static int repeated_arg_constraint(int argc, char** argv, Config* cfg);
static int empty_string_constraint(int argc, char** argv);
static int parse_inbase(char* inbaseArg, int* inbaseOut);
static int check_valid_base(char* str, int* result);
static void default_args(Config* parsedArgs);

int is_valid_letter(int digit, int base);
int is_valid_number(int digit, int base);
int is_valid_operator(int digit);
int is_valid_expr_for_base(char* expr, size_t len, int base);
int tokenizer(const char* str, const char* separators, Token** result, size_t* len);

int evaluate_expression(const char* expr, unsigned long long* result);
char* convert_any_base_to_base_ten(const char* input, int base);
char* convert_int_to_str_any_base(const char* input, int base);
char* convert_expression(const char* expr, int inputBase, int outputBase);
unsigned long long convert_str_to_any_base(const char* input, int base);

int parse_args(int argc, char** argv, Config* parsedArgs)
{

    int constraints = args_constraints(argc, argv, parsedArgs);
    if (constraints != PARSE_OK) {
        return constraints;
    }

    default_args(parsedArgs);

    char* inbaseArg;
    char* obasesArg;

    for (int x = 1; x < argc; x++) {

        if (strcmp(argv[x], "--inbase") == 0) {
            inbaseArg = argv[x + 1];
            int inbase = parse_inbase(inbaseArg, &parsedArgs->inbase);

            if (inbase != PARSE_OK) {
                return PARSE_ERROR;
            }
        }

        if (strcmp(argv[x], "--obases") == 0) {
            obasesArg = argv[x + 1];
            int obases = parse_obases(
                    obasesArg, parsedArgs->obases, &parsedArgs->numBases);

            if (obases != PARSE_OK) {
                return PARSE_ERROR;
            }
        }

        if (strcmp(argv[x], "--inputfile") == 0) {
            parsedArgs->inputfile = argv[x + 1];
        }
    }

    return PARSE_OK;
}

static void default_args(Config* parsedArgs)
{
    parsedArgs->inbase = DEFAULT_IN_BASE;
    parsedArgs->obases[0] = DEFAULT_OUT_BASE_BIN;
    parsedArgs->obases[1] = DEFAULT_OUT_BASE_DEC;
    parsedArgs->obases[2] = DEFAULT_OUT_BASE_HEX;
    parsedArgs->numBases = DEFAULT_OUT_BASE_COUNT;
    parsedArgs->inputfile = NULL;
}

static int parse_inbase(char* inbaseArg, int* inbaseOut)
{
    int isValidBase = check_valid_base(inbaseArg, inbaseOut);
    return isValidBase;
}

static int validate_obases_format(char* str)
{
    int commaFlag = 0;
    int argLen = strlen(str);

    if (argLen == 0) {
        return PARSE_ERROR;
    }

    if ((str[0] == ',') || (str[argLen - 1] == ',')) {
        return PARSE_ERROR;
    }

    for (int indx = 0; str[indx]; indx++) {
        if (!((str[indx] >= '0' && str[indx] <= '9') || str[indx] == ',')) {
            return PARSE_ERROR;
        }

        if (commaFlag && (str[indx] == ',')) {
            return PARSE_ERROR;
        }

        if (str[indx] == ',') {
            commaFlag = 1;
        } else {
            commaFlag = 0;
        }
    }

    return PARSE_OK;
}

int parse_obases(char* obaseArg, int* obaseOut, size_t* numbases)
{

    int validStr = validate_obases_format(obaseArg);

    if (validStr != PARSE_OK) {
        return PARSE_ERROR;
    }

    int counter = 0;
    int argLen = strlen(obaseArg);
    char* obaseCopy = malloc(argLen + 1);
    strcpy(obaseCopy, obaseArg);

    for (char* tok = strtok(obaseCopy, ","); tok; tok = strtok(NULL, ",")) {
        int numToken;
        int validateToken = check_valid_base(tok, &numToken);

        if (validateToken != PARSE_OK) {
            free(obaseCopy);
            return PARSE_ERROR;
        }

        obaseOut[counter] = numToken;
        counter++;
    }

    for (int x = 0; x < counter; x++) {
        for (int y = x + 1; y < counter; y++) {
            if (obaseOut[x] == obaseOut[y]) {
                free(obaseCopy);
                return PARSE_ERROR;
            }
        }
    }

    *numbases = counter;
    free(obaseCopy);
    return PARSE_OK;
}

static int args_constraints(int argc, char** argv, Config* cfg)
{

    if (argc == 1) {
        return PARSE_OK;
    }

    if (((argc - 1) % 2) != 0) {
        return PARSE_ERROR;
    }

    int repeatedArgs = repeated_arg_constraint(argc, argv, cfg);
    if (repeatedArgs != PARSE_OK) {
        return repeatedArgs;
    }

    int emptyArgs = empty_string_constraint(argc, argv);
    if (emptyArgs != PARSE_OK) {
        return emptyArgs;
    }

    return PARSE_OK;
}

static int repeated_arg_constraint(int argc, char** argv, Config* cfg)
{
    int inbaseCount = 0;
    int obaseCount = 0;
    int inputfileCount = 0;

    for (int argPos = 1; argPos < argc; argPos += 2) {
        if (strcmp(argv[argPos], "--inbase") != 0
                && strcmp(argv[argPos], "--obases") != 0
                && strcmp(argv[argPos], "--inputfile") != 0) {
            return PARSE_ERROR;
        }
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--inbase") == 0) {
            inbaseCount++;
        } else if (strcmp(argv[i], "--obases") == 0) {
            obaseCount++;
        } else if (strcmp(argv[i], "--inputfile") == 0) {
            inputfileCount++;
            if (i + 1 < argc) {
                cfg->inputfile = argv[i + 1];
            }
        }
    }

    if (inbaseCount > 1 || obaseCount > 1) {
        return PARSE_ERROR;
    }

    if (inputfileCount > 1) {
        return PARSE_FILE_ERROR;
    }

    if (inputfileCount == 1) {
        cfg->hasInputfile = 1;
    } else {
        cfg->hasInputfile = 0;
    }

    return PARSE_OK;
}

static int empty_string_constraint(int argc, char** argv)
{
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '\0') {
            return PARSE_ERROR;
        }
    }

    return PARSE_OK;
}

static int check_valid_base(char* str, int* result)
{

    if (!str || str[0] == '\0' || str[0] == '+' || str[0] == '-'
            || isspace((unsigned char)str[0])) {
        return PARSE_ERROR;
    }

    char* endptr = NULL;
    int base = DEFAULT_IN_BASE;
    long convertNum = strtol(str, &endptr, base);

    if ((endptr == str) || (*endptr != '\0')) {
        return PARSE_ERROR;
    }

    if (convertNum < MIN_BASE || convertNum > MAX_BASE) {
        return PARSE_ERROR;
    }

    *result = (int)convertNum;
    return PARSE_OK;
}

int is_valid_expr_for_base(char* expr, size_t len, int base)
{
    if (expr == NULL || expr[0] == '\0') return EXIT_ERROR;

    for(size_t i = 0; i < len; i++) {
        if(!(is_valid_letter(expr[i], base)
                || is_valid_number(expr[i], base)
                || is_valid_operator(expr[i]))){
            return EXIT_ERROR;
        }
    }

    return EXIT_OK;
}

int is_valid_letter(int digit, int base)
{
    if ('A' <= digit && digit <= 'Z') {
        if ((digit - 'A' + DEFAULT_IN_BASE) < base) {
            return EXIT_OK;
        }
    }

    if ('a' <= digit && digit <= 'z') {
        if ((digit - 'a' + DEFAULT_IN_BASE) < base) {
            return EXIT_OK;
        }
    }

    return EXIT_ERROR;
}

int is_valid_number(int digit, int base)
{
    if ('0' <= digit && digit <= '9') {
        if ((digit - '0') < base) {
            return EXIT_OK;
        }
    }

    return EXIT_ERROR;
}

int is_valid_operator(int digit)
{
    if (digit == '+' || digit == '-' || digit == '*' || digit == '/') {
        return EXIT_OK;
    }

    return EXIT_ERROR;
}

int tokenizer(const char* str, const char* separators, Token** result, size_t* len)
{
    size_t str_index = 0;
    size_t num_tokens = 0;
    Token* tokens = NULL;
    bool prev_sep = false;

    for(int i = 0; str[i] != '\0'; i++){

        // ver si es separador
        for(int j = 0; separators[j] != '\0'; j++){
            // un separador antes
            if (str[i] == separators[j]){
                // primer item del array (no hay nada)
                if (i == 0) {
                    tokens = realloc(tokens, (num_tokens + 1)*sizeof(*tokens));    
                
                    if (tokens == NULL){
                        result = NULL;
                        *len = 0;
                        return EXIT_ERROR;
                    }

                    tokens[num_tokens].value = str;
                    tokens[num_tokens].size = 1;

                    str_index += i;
                    num_tokens += 1;
                    prev_sep = true;

                    break;
                }

                // si hay un operador antes
                if (prev_sep == true){ 
                    tokens = realloc(tokens, (num_tokens + 1)*sizeof(*tokens));    
                
                    if (tokens == NULL){
                        result = NULL;
                        *len = 0;
                        return EXIT_ERROR;
                    }

                    tokens[num_tokens].value = (str + i);
                    tokens[num_tokens].size = 1;

                    str_index += i + 1;
                    num_tokens += 1;
                    prev_sep = true;

                    break;
                }

                // Cualquier otro caso 
                tokens = realloc(tokens, (num_tokens + 2)*sizeof(*tokens));
                //Agrego el token previo
                tokens[num_tokens].value = (str + str_index);
                tokens[num_tokens].size = i - str_index; 

                //Agrego el operador
                tokens[num_tokens + 1].value = (str + i);
                tokens[num_tokens + 1].size = 1;

                str_index += i + 1;
                num_tokens += 2;
                prev_sep = true;

                break;
            } else {
                // Si no es separador
                prev_sep = false;
            }
        }
    }

    *len = num_tokens;
    *result = tokens;
    return EXIT_OK;
}

/*
int evaluate_expression(const char* expr, unsigned long long* result)
{
    return 0;
}

char* convert_any_base_to_base_ten(const char* input, int base)
{
    return NULL;
}

char* convert_int_to_str_any_base(const char* input, int base)
{
    return NULL;
}

char* convert_expression(const char* expr, int inputBase, int outputBase)
{
    // setear las bases en algun lugar (10-36)
    // crear un puntero en heap con un tamano fijo dijamos 10 chars
    // iterar sobre la cadena 1 por uno
    // detectar las letras
    // cambiar las letras en nuestro heap
    return NULL;
} // Must be freed after use

unsigned long long convert_str_to_any_base(const char* input, int base)
{
    return 0;
}

*/
