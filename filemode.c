#include <stdio.h>
#include <stdlib.h>
#include <csse2310a1.h>
#include "parser.h"
#include "constants.h"
#include "outputs.h"

void print_bases_info(Config config);
void print_conversions(Config config);
int validate_expression(char* expression, int base);
void print_results(unsigned long long result, Config config);

int run_file_mode(Config config)
{

    print_greetings();
    print_bases_info(config);

    print_conversions(config);

    print_exit();
    return EXIT_OK;
}

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

        if (is_valid_expr_for_base(line, len, config.inbase) == EXIT_OK) {
            printf("Expression (base %d): %s\n", config.inbase, line);

            char* base10Expr
                    = convert_expression(line, config.inbase, DEFAULT_IN_BASE);

            if (base10Expr == NULL) {
                fprintf(stderr, "Can't evaluate the expression \"%s\"\n", line);
                continue;
            }

            unsigned long long result;
            int operation = evaluate_expression(base10Expr, &result);

            if (operation != 0) {
                fprintf(stderr, "Can't evaluate the expression \"%s\"\n", line);
                free(base10Expr);
                continue;
            }

            print_results(result, config);
            free(base10Expr);

        } else {
            fprintf(stderr, "Can't evaluate the expression \"%s\"\n", line);
        }
    }
    free(line);
    fclose(f);
}

void print_results(unsigned long long result, Config config)
{

    char* cb = convert_int_to_str_any_base(result, config.inbase);
    printf("Result (base %d): %s\n", config.inbase, cb);
    free(cb);

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
