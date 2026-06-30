#include <stdio.h>
#include <string.h>
#include "program.h"
#include "filemode.h"
#include "parser.h"
#include "constants.h"
#include "interactive.h"

static int validate_inputfile(char* dir);

/**
 * Runs the main flow of the program.
 * Steps:
 *  - parse CLI args into Config
 *  - if args are wrong, print usage to stderr
 *  - if --inputfile is given, check we can read it
 *  - (future) choose mode (stdin/file) and run
 *
 * Params:
 *  - argc, argv: command-line args
 * Returns:
 *  - PARSE_OK on success
 *  - PARSE_ERROR if arguments are invalid (usage case)
 *  - PARSE_FILE_ERROR if input file cannot be read
 */
int run_program(int argc, char** argv)
{
    Config config = {0};

    // 1. PARSING
    // Chech arguments
    int parsing = parse_args(argc, argv, &config);
    if (parsing == PARSE_ERROR) {
        fputs("Usage: ./basejump [--inbase 2..36] [--obases 2..36] "
              "[--inputfile string]\n",
                stderr);
        return parsing;
    }

    if (parsing == PARSE_FILE_ERROR) {
        fprintf(stderr, "basejump: unable to read from input file \"%s\"\n",
                config.inputfile);
        return parsing;
    }

    // Check if --inputfile exist
    int inputfile = validate_inputfile(config.inputfile);
    if (inputfile == PARSE_FILE_ERROR) {
        fprintf(stderr, "basejump: unable to read from input file \"%s\"\n",
                config.inputfile);
        return inputfile;
    }

    // 2. VERIFY MODE
    // FILE MODE
    if (config.hasInputfile) {
        if (inputfile == PARSE_FILE_ERROR) {
            fprintf(stderr,
                    "basejump: unable to read from input file \"%s\"\n",
                    config.inputfile);
            return inputfile;
        }

        // FILE MODE
        run_file_mode(config);
    } else {
        // INTERACTIVE MODE
        run_interactive_mode(&config);
    }

    // 3. END
    return 0;
}

/**
 * Quick check for the input file.
 * If dir is NULL, it is OK.
 * If dir is not NULL, try fopen(dir, "r"):
 *   - success -> PARSE_OK
 *   - fail    -> PARSE_FILE_ERROR
 *
 * Params:
 *  - dir: file path or NULL
 * Returns:
 *  - PARSE_OK or PARSE_FILE_ERROR if file cannot be read
 */
static int validate_inputfile(char* dir)
{
    if (dir) {
        if (strncmp(dir, "--", 2) == 0) {
            return PARSE_FILE_ERROR;
        }

        FILE* f = fopen(dir, "r");
        if (!f) {
            return PARSE_FILE_ERROR;
        }

        fclose(f);
    }

    return PARSE_OK;
}
