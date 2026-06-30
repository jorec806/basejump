#include <stdio.h>
#include <stdlib.h>
#include <csse2310a1.h>
#include "parser.h"
#include "constants.h"
#include "outputs.h"

void print_bases_info(Config config);
void print_conversions(Config config);
int validate_expression(char* expression, int base);
int valid_digit_for_base(int digit, int base);
void print_results(unsigned long long result, Config config);

/**
 * Runs file mode: prints greeting/base info, processes each input line
 * (validate → convert to base 10 → evaluate → print).
 *
 * Params:
 *  - config: inbase, obases[0..numBases-1], inputfile.
 * Returns:
 *  - EXIT_OK.
 */
int run_file_mode(Config config)
{
    // Initial info: greeting and bases
    print_greetings();
    print_bases_info(config);
    // Read file and print conversions
    print_conversions(config);

    // End program
    print_exit();
    return EXIT_OK;
}

/* print_conversions()
 * --------------------
 * Opens config.inputfile, reads each line, trims the trailing newline, checks
 * that the expression is valid for the input base, converts it to base 10,
 * evaluates it, and prints results in input and output bases.
 *
 * config: execution configuration (inbase, obases[0..numBases-1], numBases,
 *         inputfile).
 *
 * Returns: nothing
 *
 * Errors: if the file cannot be opened, the function returns immediately
 *         without output; if conversion/evaluation fails for a line, prints
 *         exactly: Can't evaluate the expression "<line>"
 *         to stderr and continues with the next line.
 */
void print_conversions(Config config)
{
    FILE* f = fopen(config.inputfile, "r");
    if (!f) {
        return;
    }

    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, f)) != -1) {
        if (read > 0 && line[read - 1] == '\n') {
            line[read - 1] = '\0';
            read--;
        }

        if (validate_expression(line, config.inbase) == EXIT_OK) {
            printf("Expression (base %d): %s\n", config.inbase, line);

            char* base10Expr
                    = convert_expression(line, config.inbase, DEFAULT_IN_BASE);

            if (base10Expr == NULL) {
                fprintf(stderr, "Can't evaluate the expression \"%s\"\n", line);
                continue;
            }

            unsigned long long result;
            int operation = evaluate_expression(base10Expr, &result);

            // if operation was not succesful
            if (operation != 0) {
                fprintf(stderr, "Can't evaluate the expression \"%s\"\n", line);
                free(base10Expr);
                continue;
            }

            // Print all conversions for every base
            print_results(result, config);
            free(base10Expr);

        } else {
            fprintf(stderr, "Can't evaluate the expression \"%s\"\n", line);
        }
    }
    free(line);
    fclose(f);
}

/* print_results()
 * ----------------
 * Prints the evaluated result in the input base and then in each output base.
 *
 * result: evaluated value (0..2^64-1).
 * config: execution configuration (inbase, obases[0..numBases-1], numBases).
 *
 * Returns: nothing
 *
 * Errors: if a base conversion returns NULL for a given base, prints a brief
 *         "(base X) ERROR" marker for that base and continues.
 */
void print_results(unsigned long long result, Config config)
{
    // Print result in input base
    char* cb = convert_int_to_str_any_base(result, config.inbase);
    printf("Result (base %d): %s\n", config.inbase, cb);
    free(cb);

    // Print conversions for all output bases
    for (int i = 0; i < (int)config.numBases; i++) {
        char* converted = convert_int_to_str_any_base(result, config.obases[i]);
        if (converted) {
            printf("Base %d: %s\n", config.obases[i], converted);
            free(converted);
        } else {
            printf("(base %d) ERROR", config.obases[i]);
        }
    }
}

/* validate_expression()
 * ----------------------
 * Checks that 'expression' is syntactically acceptable for 'base':
 * - first character is a valid digit/letter for 'base'
 * - only '+', '-', '*', '/' as binary operators
 * - no consecutive operators
 * - all digits/letters are valid for 'base'
 *
 * expression: NUL-terminated string; not NULL.
 * base: integer base in [2..36].
 *
 * Returns: EXIT_OK if valid; EXIT_ERROR otherwise
 */
int validate_expression(char* expression, int base)
{
    int signFlag = 0;
    if (valid_digit_for_base(expression[0], base) == EXIT_ERROR) {
        return EXIT_ERROR;
    }

    for (int x = 0; expression[x] != '\0'; x++) {
        char digit = expression[x];

        if (digit == '+' || digit == '-' || digit == '*' || digit == '/') {
            if (signFlag == 1) {
                return EXIT_ERROR;
            }

            signFlag = 1;
            continue;
        }

        if ((valid_digit_for_base(digit, base)) == EXIT_ERROR) {
            return EXIT_ERROR;
        }

        signFlag = 0;
    }

    return EXIT_OK;
}

/* valid_digit_for_base()
 * -----------------------
 * Determines whether a single character is a valid digit for 'base' using
 * ASCII ranges ('0'-'9', 'A'-'Z', 'a'-'z'); maps A/a→10, B/b→11, etc., and
 * checks value < base.
 *
 * digit: character to test.
 * base: integer base in [2..36].
 *
 * Returns: EXIT_OK if the character is valid for 'base'; EXIT_ERROR otherwise
 *
 * REF: ASCII mapping idea discussed with ChatGPT (toolHistory.txt, 2025-08-25)
 */
int valid_digit_for_base(int digit, int base)
{
    if ('0' <= digit && digit <= '9') {
        if ((digit - '0') < base) {
            return EXIT_OK;
        }
    }

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
