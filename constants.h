
#ifndef CONSTANTS_H
#define CONSTANTS_H

#define BJ_USAGE_ERROR "Usage: ./basejump [--inbase 2..36] [--obases 2..36] [--inputfile string]"

enum { PARSE_OK = 0, PARSE_ERROR = 4, PARSE_FILE_ERROR = 18 };

enum { EXIT_OK = 0, EXIT_ERROR = 1 };

enum { KEY_BACKSPACE = 127, KEY_ESCAPE = 27, KEY_ENTER = 10 };

enum {
    EOT = 4,
    MAX_INPUT_SIZE = 64,
    MIN_BASE = 2,
    MAX_BASE = 36,
    DEFAULT_IN_BASE = 10,
    DEFAULT_OUT_BASE_BIN = 2,
    DEFAULT_OUT_BASE_DEC = 10,
    DEFAULT_OUT_BASE_HEX = 16,
    DEFAULT_OUT_BASE_COUNT = 3,
};

typedef struct History {
    char* expression;
    char* result;
    int inbase;
} History;

#endif
