/**
 * Centralised constants for basejump: exit/status codes and base
 * ranges/defaults.
 *
 * Purpose:
 *  - Avoid magic numbers and duplication across modules.
 *  - Keep a single source of truth for values used by parser.c/program.c.
 *
 * REF: ChatGPT suggested extracting base range/defaults and status codes
 * REF: into this dedicated header (constants.h) to eliminate magic numbers
 * REF: and improve consistency across files. See toolHistory.txt (2025-08-24).
 */
#ifndef CONSTANTS_H
#define CONSTANTS_H

// Parse Exit types
enum { PARSE_OK = 0, PARSE_ERROR = 4, PARSE_FILE_ERROR = 18 };

// Program Exit types
enum { EXIT_OK = 0, EXIT_ERROR = 1 };

// Special Keys ASCII
enum { KEY_BACKSPACE = 127, KEY_ESCAPE = 27, KEY_ENTER = 10 };

// Default values for basejump
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

// Chatgpt suggested the use of struct to store a new history entry
typedef struct History {
    char* expression;
    char* result;
    int inbase;
} History;

#endif
