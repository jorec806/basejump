#include <stdio.h>
#include <string.h>
#include "program.h"
#include "filemode.h"
#include "parser.h"
#include "constants.h"
#include "interactive.h"

static int validate_inputfile(char* dir);

int run_program(int argc, char** argv)
{
    Config config = {0};

    int parsing = parse_args(argc, argv, &config);
    if (parsing == PARSE_ERROR) {
	/*
	fputs("Usage: ./basejump [--inbase 2..36] [--obases 2..36] "
              "[--inputfile string]\n",
                stderr);
	*/
	fprintf(stderr, "%s" , BJ_USAGE_ERROR);
        return parsing;
    }

    if (parsing == PARSE_FILE_ERROR) {
        fprintf(stderr, "basejump: unable to read from input file \"%s\"\n",
                config.inputfile);
        return parsing;
    }

    int inputfile = validate_inputfile(config.inputfile);
    if (inputfile == PARSE_FILE_ERROR) {
        fprintf(stderr, "basejump: unable to read from input file \"%s\"\n",
                config.inputfile);
        return inputfile;
    }

    if (config.hasInputfile) {
        if (inputfile == PARSE_FILE_ERROR) {
            fprintf(stderr,
                    "basejump: unable to read from input file \"%s\"\n",
                    config.inputfile);
            return inputfile;
        }

        run_file_mode(config);
    } else {

        run_interactive_mode(&config);
    }

    return 0;
}

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
