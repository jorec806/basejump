/*
 * References:
 * - OpenAI ChatGPT (GPT-5 Thinking), 23 Aug 2025.
 *   Assistance: CLI parsing edge cases (argv spacing, exact strtol checks),
 *   Usage-on-stderr+exit codes, test cases, and toolHistory format template.
 *   Integration: guidance only; I wrote the code.
 *   Affected areas: parse_args(), check_valid_base(), obases parsing,
 */

#include <stdio.h>
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

/**
 * Parses CLI options (--inbase, --obases, --inputfile) and fills parsedArgs.
 * Applies defaults via default_args() when no user arguments are supplied.
 *
 * Params:
 *  - argc, argv: command-line arguments.
 *  - parsedArgs: output; not NULL.
 * Returns:
 *  - PARSE_OK on success; PARSE_ERROR if any constraint is violated
 *    (missing pairs, repeated/unknown options, invalid values).
 */
int parse_args(int argc, char** argv, Config* parsedArgs)
{
    // 1. Check all constraints
    int constraints = args_constraints(argc, argv, parsedArgs);
    if (constraints != PARSE_OK) {
        return constraints;
    }

    // 2. Populate Config
    // DEFAULT
    default_args(parsedArgs);

    char* inbaseArg;
    char* obasesArg;

    for (int x = 1; x < argc; x++) {
        // INBASE
        if (strcmp(argv[x], "--inbase") == 0) {
            inbaseArg = argv[x + 1];
            int inbase = parse_inbase(inbaseArg, &parsedArgs->inbase);

            if (inbase != PARSE_OK) {
                return PARSE_ERROR;
            }
        }

        // OBASES
        if (strcmp(argv[x], "--obases") == 0) {
            obasesArg = argv[x + 1];
            int obases = parse_obases(
                    obasesArg, parsedArgs->obases, &parsedArgs->numBases);

            if (obases != PARSE_OK) {
                return PARSE_ERROR;
            }
        }

        // INPUTFILE
        if (strcmp(argv[x], "--inputfile") == 0) {
            parsedArgs->inputfile = argv[x + 1];
        }
    }

    return PARSE_OK;
}

/**
 * Sets default values in parsedArgs when no user arguments are provided.
 *
 * Params:
 *  - argc: number of arguments.
 *  - parsedArgs: output; not NULL.
 */
static void default_args(Config* parsedArgs)
{
    parsedArgs->inbase = DEFAULT_IN_BASE;
    parsedArgs->obases[0] = DEFAULT_OUT_BASE_BIN;
    parsedArgs->obases[1] = DEFAULT_OUT_BASE_DEC;
    parsedArgs->obases[2] = DEFAULT_OUT_BASE_HEX;
    parsedArgs->numBases = DEFAULT_OUT_BASE_COUNT;
    parsedArgs->inputfile = NULL;
}

/**
 * Validates and converts inbaseArg to a base-10 integer in
 * [MIN_BASE..MAX_BASE].
 *
 * Params:
 *  - inbaseArg: input string; not NULL.
 *  - inbaseOut: output integer; not NULL.
 * Returns:
 *  - PARSE_OK if valid; PARSE_ERROR otherwise.
 */
static int parse_inbase(char* inbaseArg, int* inbaseOut)
{
    int isValidBase = check_valid_base(inbaseArg, inbaseOut);
    return isValidBase;
}

/**
 * Checks that str is a comma-separated list of decimal integers:
 * - only digits and ',' characters,
 * - no leading/trailing comma,
 * - no empty fields (no consecutive commas),
 * - not empty.
 *
 * Params:
 *  - str: input string; not NULL.
 * Returns:
 *  - PARSE_OK if the format is valid; PARSE_ERROR otherwise.
 */
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

/**
 * Parses a comma-separated list of bases, validates range [MIN_BASE..MAX_BASE],
 * rejects duplicates, and writes the results and count.
 *
 * Params:
 *  - obaseArg: input string; not NULL.
 *  - obaseOut: output array of bases; not NULL.
 *  - numbases: number of bases written to obaseOut; not NULL.
 * Returns:
 *  - PARSE_OK on success; PARSE_ERROR on invalid format, out-of-range value,
 *    or duplicate entries.
 */
int parse_obases(char* obaseArg, int* obaseOut, size_t* numbases)
{
    // Validate str with only "," or digits
    int validStr = validate_obases_format(obaseArg);

    if (validStr != PARSE_OK) {
        return PARSE_ERROR;
    }

    // Generate array with all bases
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

    // Check for duplicate numbers
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

/**
 * Validates overall argument structure:
 * - allows argc==1,
 * - requires (option,value) pairs,
 * - forbids unknown or repeated options,
 * - forbids empty-string arguments.
 *
 * Params:
 *  - argc, argv: command-line arguments.
 * Returns:
 *  - PARSE_OK if all checks pass; PARSE_ERROR otherwise.
 */
static int args_constraints(int argc, char** argv, Config* cfg)
{
    // Check if there are no arguments
    if (argc == 1) {
        return PARSE_OK;
    }

    // Check if every arg has a pair
    if (((argc - 1) % 2) != 0) {
        return PARSE_ERROR;
    }

    // Check repeated Option Args
    int repeatedArgs = repeated_arg_constraint(argc, argv, cfg);
    if (repeatedArgs != PARSE_OK) {
        return repeatedArgs;
    }

    // Check empty String Args
    int emptyArgs = empty_string_constraint(argc, argv);
    if (emptyArgs != PARSE_OK) {
        return emptyArgs;
    }

    return PARSE_OK;
}

/**
 * Ensures each allowed option (--inbase, --obases, --inputfile) appears at
 * most once and that no unknown options are present (expects option/value
 * pairs).
 *
 * Params:
 *  - argc, argv: command-line arguments.
 * Returns:
 *  - PARSE_OK if no repetitions/unknown options; PARSE_ERROR otherwise.
 */
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

/**
 * Checks that no argument equals the empty string "".
 *
 * Params:
 *  - argc, argv: command-line arguments.
 * Returns:
 *  - PARSE_OK if all are non-empty; PARSE_ERROR otherwise.
 */
static int empty_string_constraint(int argc, char** argv)
{
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '\0') {
            return PARSE_ERROR;
        }
    }

    return PARSE_OK;
}

/**
 * Validates that str is a complete decimal integer in [MIN_BASE..MAX_BASE]
 * with no leading sign and no leading/trailing whitespace; writes result.
 * Performs a quick pre-check for leading '+'/'-' or whitespace (via isspace)
 * before calling strtol.
 *
 * Params:
 *  - str: input string (may be NULL).
 *  - result: output integer; not NULL when expecting PARSE_OK.
 * Returns:
 *  - PARSE_OK if valid; PARSE_ERROR if NULL/empty, signed/whitespace-leading,
 *    non-numeric, or out of range.
 *
 * REF: Suggestion to pre-check leading sign/whitespace using isspace() before
 * REF: strtol came from ChatGPT (see toolHistory.txt, 2025-08-24).
 */
static int check_valid_base(char* str, int* result)
{
    // check if it is numeric base 10
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
