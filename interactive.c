#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <csse2310a1.h>
#include "interactive.h"
#include "parser.h"
#include "outputs.h"
#include "constants.h"
#include "filemode.h"

static int validate_char(char inputChar, Config* cfg);
static int valid_operand(int inputChar);
static int valid_special_char(int inputChar);
static int handle_char(int inputChar, Config* cfg, char* inputBuffer,
        char** expBuffer, int* expBuffSize);
static int handle_digit(int inputChar, char* inputBuffer);
static int handle_operator(
        int inputChar, char* inputBuffer, char** expBuffer, int* expBuffSize);
static int handle_special_char(
        int inputChar, char* inputBuffer, char** expBuffer, int* expBuffSize);
static int handle_new_line(
        int inputChar, char* inputBuffer, char** expBuffer, int* expBuffSize);
static int handle_printing(int inputChar, Config* cfg, char* inputBuffer,
        char** expBuffer, int* expBuffSize);
static int handle_escape(
        int inputChar, char* inputBuffer, char** expBuffer, int* expBuffSize);
static int handle_backspace(int inputChar, char* inputBuffer);
static int handle_command(char inputChar, Config* cfg, char* commBuff,
        int* commandMode, char* inputBuffer, char** expBuffer,
        int* expBufferSize);
static int command_inbase(char* inbase, Config* cfg);
static int parse_command(char* command, Config* cfg);
static int command_history(
        Config* cfg, char* expression, char* result, int inbase);
static void print_history(Config* cfg);
static void start_session(Config* cfg);
static void end_session(Config* cfg, char** expBuffer);

/* run_interactive_mode()
 * -----------------------
 * Runs interactive mode: clears screen, prints greeting/bases/instructions,
 * disables line buffering, reads characters until EOF or Ctrl-D, routes input
 * through handlers to build/print expressions and results, then restores
 * line buffering and exits.
 *
 * cfg: execution configuration (inbase, obases[0..numBases-1], numBases,
 *      inputfile).
 *
 * Returns: EXIT_OK
 */
int run_interactive_mode(Config* cfg)
{
    start_session(cfg);

    // Buffers
    char inputBuffer[MAX_INPUT_SIZE + 1];
    char* expBuffer = malloc(sizeof(*expBuffer));
    int expBufferSize = 1;
    inputBuffer[0] = '\0';
    expBuffer[0] = '\0';

    int inputChar;
    int commandMode = 0;
    char commBuffer[MAX_INPUT_SIZE + 1];
    commBuffer[0] = '\0';

    while ((inputChar = fgetc(stdin)) != EOF && inputChar != EOT) {
        if (inputChar == '\n') {
            if (strcmp(commBuffer, "h") == 0) {
                print_history(cfg);
                commBuffer[0] = '\0';
                commandMode = 0;
                continue;
            }
        }

        if (commandMode || inputChar == ':') {
            handle_command(inputChar, cfg, commBuffer, &commandMode,
                    inputBuffer, &expBuffer, &expBufferSize);
            if (!commandMode) {
                handle_printing(
                        '\0', cfg, inputBuffer, &expBuffer, &expBufferSize);
            }
            continue;
        }

        handle_char(inputChar, cfg, inputBuffer, &expBuffer, &expBufferSize);
        handle_printing(
                inputChar, cfg, inputBuffer, &expBuffer, &expBufferSize);
    }

    end_session(cfg, &expBuffer);

    return EXIT_OK;
}

/* start_session()
 * ----------------
 * Clears the screen, prints greetings/base info/instructions, and disables
 * line buffering to enable character-by-character input.
 *
 * cfg: execution configuration (inbase, obases[0..numBases-1], numBases).
 *
 * Returns: nothing
 */
static void start_session(Config* cfg)
{
    clear_screen();
    print_greetings();
    print_bases_info(*cfg);
    print_instruction();
    disable_line_buffering();
}

/* end_session()
 * ---------------
 * Restores line buffering, frees all history entries and the history array,
 * frees the expression buffer, prints the exit message.
 *
 * cfg: execution configuration (provides history and its size).
 * expBuffer: pointer to expression buffer to free.
 *
 * Returns: nothing
 */
static void end_session(Config* cfg, char** expBuffer)
{
    enable_line_buffering();

    // Memory free
    for (int i = 0; i < cfg->historySize; i++) {
        free(cfg->history[i].expression);
        free(cfg->history[i].result);
    }

    free(cfg->history);
    free(*expBuffer);

    print_exit();
}

/* handle_command()
 * -----------------
 * Handles "command mode". A leading ':' enters command mode; characters are
 * accumulated into commBuff until newline, then parse_command() is invoked.
 * On success, clears buffers and exits command mode; on failure, exits with
 * error.
 *
 * inputChar: character just read (may be ':' or part of the command).
 * cfg: execution configuration to be updated by commands.
 * commBuff: command buffer (NUL-terminated, max MAX_INPUT_SIZE).
 * commandMode: in/out flag (1 while in command mode, 0 otherwise).
 * inputBuffer: numeric input buffer (may be cleared on success).
 * expBuffer: pointer to expression buffer (may be reset on success).
 * expBuffSize: pointer to allocated size counter for *expBuffer (may reset).
 *
 * Returns: EXIT_OK on handled input; EXIT_ERROR if parse_command() fails.
 */
static int handle_command(char inputChar, Config* cfg, char* commBuff,
        int* commandMode, char* inputBuffer, char** expBuffer,
        int* expBufferSize)
{

    if (inputChar == ':') {
        *commandMode = 1;
        return EXIT_OK;
    }

    // If in Command Mode
    if (*commandMode == 1) {
        if (inputChar == '\n') {
            int parsedCommand = parse_command(commBuff, cfg);
            if (parsedCommand == EXIT_OK) {
                inputBuffer[0] = '\0';
                commBuff[0] = '\0';
                *expBuffer[0] = '\0';
                *expBufferSize = 1;
                *commandMode = 0;

                // reimprimir con nueva informacion (abrir canal de impresion)
            } else {
                *commandMode = 0;
                return EXIT_ERROR;
            }
        } else {
            // keep adding inputchar to buffer (remember the BUFFER LIMIT!)
            int len = strlen(commBuff);
            if (len < MAX_INPUT_SIZE) {
                commBuff[len] = inputChar;
                commBuff[len + 1] = '\0';
            }
        }
    }

    return EXIT_OK;
}

/* print_history()
 * ----------------
 * Clears the screen and prints each saved expression/result pair from the
 * session history in the base used when evaluated.
 *
 * cfg: execution configuration providing history entries and size.
 *
 */
static void print_history(Config* cfg)
{
    clear_screen();

    for (int x = 0; x < cfg->historySize; x++) {
        printf("Expression (base %d): %s\n", cfg->history[x].inbase,
                cfg->history[x].expression);
        printf("Result (base %d): %s\n", cfg->history[x].inbase,
                cfg->history[x].result);
    }
}

/* parse_command()
 * ----------------
 * Parses and applies a command from the form:
 *   i<base>    → set input base
 *   o<b1,b2,..>→ set output bases (comma-separated)
 * Validates format and delegates to command_inbase() / parse_obases().
 *
 * command: NUL-terminated command string (e.g., "i16", "o2,10,16").
 * cfg: execution configuration to update.
 *
 * Returns: EXIT_OK on success; EXIT_ERROR on invalid format, unknown command,
 *          or invalid base(s).
 */
static int parse_command(char* command, Config* cfg)
{
    if (strlen(command) < 2) {
        return EXIT_ERROR;
    }

    char tempBuff[MAX_INPUT_SIZE + 1];
    strncpy(tempBuff, command + 1, strlen(command) - 1);
    tempBuff[strlen(command) - 1] = '\0';

    if (strlen(tempBuff) < 1) {
        return EXIT_ERROR;
    }

    int okInbase = 0;
    int okObases = 0;
    // int okHistoty;

    if (command[0] == 'i') {
        okInbase = command_inbase(tempBuff, cfg);
    } else if (command[0] == 'o') {
        okObases = parse_obases(tempBuff, cfg->obases, &(cfg->numBases));
    } else {
        return EXIT_ERROR;
    }

    if (okInbase == EXIT_ERROR || okObases == EXIT_ERROR) {
        return EXIT_ERROR;
    }

    return EXIT_OK;
}

/* command_history()
 * ------------------
 * Appends a new entry to the session history, resizing the history array and
 * deep-copying the expression and result strings.
 *
 * cfg: execution configuration holding history array/size (updated).
 * expression: expression string to store (NUL-terminated).
 * result: result string to store (NUL-terminated).
 * inbase: base used for the evaluation (stored with the entry).
 *
 * Returns: EXIT_OK on success; EXIT_ERROR if allocation/reallocation fails.
 *
 * REF: Discussion on struct-pointer vs copy and '->' usage with ChatGPT
 * REF: (toolHistory.txt, 2025-08-28).
 */
static int command_history(
        Config* cfg, char* expression, char* result, int inbase)
{

    // Resizing of collection of History
    cfg->historySize++;
    History* temp = realloc(cfg->history, sizeof(History) * cfg->historySize);

    if (temp == NULL) {
        cfg->historySize--;
        return EXIT_ERROR;
    }

    cfg->history = temp;

    // Calculate new sizes for History members
    int index = cfg->historySize - 1; // index of new entry
    cfg->history[index].expression = malloc(strlen(expression) + 1);
    cfg->history[index].result = malloc(strlen(result) + 1);

    if (cfg->history[index].expression == NULL
            || cfg->history[index].result == NULL) {
        return EXIT_ERROR;
    }

    // Populate new History item
    strcpy(cfg->history[index].expression, expression);
    strcpy(cfg->history[index].result, result);
    cfg->history[index].inbase = inbase;

    return EXIT_OK;
}

/* command_inbase()
 * -----------------
 * Validates and applies a new input base from a decimal string (length ≤ 2),
 * ensuring MIN_BASE ≤ base ≤ MAX_BASE, then updates cfg->inbase.
 *
 * inbase: decimal string representing the new base (e.g., "2", "16", "36").
 * cfg: execution configuration to update.
 *
 * Returns: EXIT_OK on success; EXIT_ERROR if non-numeric, too long, or out
 *          of allowed range.
 */
static int command_inbase(char* inbase, Config* cfg)
{

    // Check if lenght is more than 2 digits (max base 36)
    if (strlen(inbase) > 2) {
        return EXIT_ERROR;
    }

    // Check if every char is numerical
    for (int x = 0; inbase[x] != '\0'; x++) {
        if (!(inbase[x] >= '0' && inbase[x] <= '9')) {
            return EXIT_ERROR;
        }
    }

    // Convert to int
    char* endptr;
    long newBase = strtol(inbase, &endptr, DEFAULT_IN_BASE); // Base 10

    // check if 2< inbase <36
    if (newBase < MIN_BASE || newBase > MAX_BASE) {
        return EXIT_ERROR;
    }

    cfg->inbase = (int)newBase;

    return EXIT_OK;
}

/* handle_printing()
 * ------------------
 * Renders the current state to stdout. Always prints the expression. If the
 * last key was newline, converts and evaluates the completed expression, then
 * prints the result in the input base and in each output base; otherwise,
 * prints the current input buffer and live conversions.
 *
 * inputChar: last character read.
 * cfg: execution configuration (inbase, obases[0..numBases-1], numBases).
 * inputBuffer: current numeric buffer being typed (NUL-terminated).
 * expBuffer: pointer to the expression buffer (built across operators).
 * expBuffSize: pointer to current allocated size for *expBuffer.
 *
 * Returns: EXIT_OK on normal printing; EXIT_ERROR if evaluation of a completed
 *          expression failed (no result printed).
 */
static int handle_printing(int inputChar, Config* cfg, char* inputBuffer,
        char** expBuffer, int* expBuffSize)
{
    // Calc expression base N to base 10
    unsigned long long result = 0;
    char* expr10 = convert_expression(*expBuffer, cfg->inbase, DEFAULT_IN_BASE);
    int isExpValid = evaluate_expression(expr10, &result);

    if ((inputChar == '\n') && (isExpValid != EXIT_OK)) {
        fprintf(stderr, "Can't evaluate the expression \"%s\"\n", *expBuffer);
        (*expBuffer)[0] = '\0';
        *expBuffSize = 1;
        inputBuffer[0] = '\0';
        free(expr10);
        return EXIT_ERROR;
    }

    clear_screen();
    if (inputChar == '\n') {
        char* exprInbase = convert_int_to_str_any_base(result, cfg->inbase);
        printf("Expression (base %d): %s\n", cfg->inbase, *expBuffer);
        printf("Result (base %d): %s\n", cfg->inbase, exprInbase);

        command_history(cfg, *expBuffer, exprInbase, cfg->inbase);
        (*expBuffer)[0] = '\0';
        *expBuffSize = 1;
        free(exprInbase);
    } else {
        printf("Expression (base %d): %s\n", cfg->inbase, *expBuffer);
        printf("Input (base %d): %s\n", cfg->inbase, inputBuffer);
    }

    for (int x = 0; x < (int)cfg->numBases; x++) {
        char* c = NULL;
        if (inputChar == '\n') {
            c = convert_int_to_str_any_base(result, cfg->obases[x]);
        } else {
            unsigned long long num
                    = convert_str_to_int_any_base(inputBuffer, cfg->inbase);
            c = convert_int_to_str_any_base(num, cfg->obases[x]);
        }
        printf("Base %d: %s\n", cfg->obases[x], c);
        free(c);
    }

    if (inputChar == '\n') {
        inputBuffer[0] = '\0';
    }
    free(expr10);
    return EXIT_OK;
}

/* handle_char()
 * --------------
 * Validates the incoming character and dispatches it to the appropriate
 * handler: digit (append to input buffer), operator (append operator and
 * flush input into expression), or special character (e.g., newline).
 *
 * inputChar: character to process.
 * cfg: execution configuration (inbase for digit validation).
 * inputBuffer: current numeric buffer being typed (NUL-terminated).
 * expBuffer: pointer to the expression buffer (to be grown/updated).
 * expBuffSize: pointer to current allocated size for *expBuffer.
 *
 * Returns: EXIT_OK if character was accepted/processed; EXIT_ERROR if invalid.
 */
static int handle_char(int inputChar, Config* cfg, char* inputBuffer,
        char** expBuffer, int* expBuffSize)
{
    if (validate_char(inputChar, cfg) != EXIT_OK) {
        return EXIT_ERROR;
    }

    if (valid_digit_for_base(inputChar, cfg->inbase) == EXIT_OK) {
        return handle_digit(inputChar, inputBuffer);
    }

    if (valid_operand(inputChar) == EXIT_OK) {
        return handle_operator(inputChar, inputBuffer, expBuffer, expBuffSize);
    }

    if (valid_special_char(inputChar) == EXIT_OK) {
        return handle_special_char(
                inputChar, inputBuffer, expBuffer, expBuffSize);
    }

    return EXIT_OK;
}

/* handle_digit()
 * ---------------
 * Appends a single (already validated) digit to the input buffer provided it
 * does not exceed MAX_INPUT_SIZE.
 *
 * inputChar: digit to append.
 * inputBuffer: target buffer (NUL-terminated).
 *
 * Returns: EXIT_OK on success; EXIT_ERROR if the buffer is full.
 */
static int handle_digit(int inputChar, char* inputBuffer)
{
    int len = strlen(inputBuffer);

    if (len < MAX_INPUT_SIZE) {
        inputBuffer[len] = inputChar;
        inputBuffer[len + 1] = '\0';
        return EXIT_OK;
    }

    return EXIT_ERROR;
}

/* handle_operator()
 * ------------------
 * Appends an operator to the input buffer (inserting a leading '0' if the
 * buffer is empty), then concatenates the input buffer into the expression
 * buffer (growing it via realloc) and clears the input buffer.
 *
 * inputChar: operator to append ('+', '-', '*', '/').
 * inputBuffer: current numeric buffer (NUL-terminated).
 * expBuffer: pointer to the expression buffer to extend.
 * expBuffSize: pointer to allocated size counter for *expBuffer (updated).
 *
 * Returns: EXIT_OK on success; EXIT_ERROR if memory reallocation fails.
 */
static int handle_operator(
        int inputChar, char* inputBuffer, char** expBuffer, int* expBuffSize)
{
    // Add operator to inputBuffer
    char* tempBuffer;

    // Validate if empty buffer
    if (strlen(inputBuffer) == 0) {
        inputBuffer[0] = '0';
        // inputBuffer[1] = inputChar;
        inputBuffer[1] = '\0';

        // 1 for '0', 1 for operator
        *expBuffSize += 1 + 1;
    } else {
        int len = strlen(inputBuffer);
        /*
        inputBuffer[len] = inputChar;
        inputBuffer[len + 1] = '\0';
        */
        // inputBuffer length + operator
        *expBuffSize += len + 1;
    }

    tempBuffer = realloc(*expBuffer, sizeof(char) * (*expBuffSize));
    if (tempBuffer == NULL) {
        return EXIT_ERROR;
    }

    *expBuffer = tempBuffer;
    strcat(*expBuffer, inputBuffer);
    char operator[2] = {(char)inputChar, '\0' };
    strcat(*expBuffer, operator);

    // Clean inputBuffer
    inputBuffer[0] = '\0';

    return EXIT_OK;
}

/* handle_special_char()
 * ----------------------
 * Handles special keys. Currently supports newline by finalizing the current
 * term into the expression buffer; other cases are placeholders for future
 * features (backspace, escape, commands).
 *
 * inputChar: special character received (e.g., '\n').
 * inputBuffer: current numeric buffer (may be consumed on newline).
 * expBuffer: pointer to the expression buffer to extend.
 * expBuffSize: pointer to allocated size counter for *expBuffer (updated).
 *
 * Returns: EXIT_OK on handled key; EXIT_ERROR for unrecognized specials.
 *
 * REF: Switch-based dispatch suggested by OpenAI ChatGPT (GPT-5 Thinking),
 *      toolHistory.txt (2025-08-25).
 */
static int handle_special_char(
        int inputChar, char* inputBuffer, char** expBuffer, int* expBuffSize)
{
    switch (inputChar) {
    case '\n':
        return handle_new_line(inputChar, inputBuffer, expBuffer, expBuffSize);
    case KEY_BACKSPACE:
        return handle_backspace(inputChar, inputBuffer);
    case KEY_ESCAPE:
        return handle_escape(inputChar, inputBuffer, expBuffer, expBuffSize);
    default:
        return EXIT_ERROR;
    }
}

/* handle_new_line()
 * ------------------
 * Finalizes the current term on newline: ensures inputBuffer is non-empty
 * (inserts "0" if empty), grows the expression buffer, concatenates the input,
 * clears inputBuffer, and signals success.
 *
 * inputChar: expected to be '\n'.
 * inputBuffer: current numeric buffer (consumed).
 * expBuffer: pointer to the expression buffer to extend.
 * expBuffSize: pointer to allocated size counter for *expBuffer (updated).
 *
 * Returns: EXIT_OK when newline is processed; EXIT_ERROR otherwise or if
 *          memory reallocation fails.
 */
static int handle_new_line(
        int inputChar, char* inputBuffer, char** expBuffer, int* expBuffSize)
{
    if (inputChar == '\n') {
        // If expression buffer is not empty
        if (*expBuffSize >= 2) {
            // If input buffer empty
            if (strlen(inputBuffer) == 0) {
                inputBuffer[0] = '0';
                inputBuffer[1] = '\0';
            }
            // If expression buffer is empty
        } else {
            // If input buffer is empty
            if (strlen(inputBuffer) == 0) {
                inputBuffer[0] = '0';
                inputBuffer[1] = '\0';
            }
        }

        int len = strlen(inputBuffer);
        *expBuffSize += len;
        char* tempBuffer = realloc(*expBuffer, sizeof(char) * (*expBuffSize));

        if (tempBuffer == NULL) {
            return EXIT_ERROR;
        }

        *expBuffer = tempBuffer;
        strcat(*expBuffer, inputBuffer);

        /*
        // Clean inputBuffer
        inputBuffer[0] = '\0';
        */

        return EXIT_OK;
    }

    return EXIT_ERROR;
}

/* handle_escape()
 * ----------------
 * Resets both input and expression buffers to empty strings when escape key
 * (ASCII 27) is pressed. Reallocates expression buffer to minimal size.
 *
 * inputChar: expected to be escape key (27).
 * inputBuffer: current numeric buffer to clear.
 * expBuffer: pointer to expression buffer to reset.
 * expBuffSize: pointer to allocated size counter (reset to 1).
 *
 * Returns: EXIT_OK on successful reset; EXIT_ERROR if realloc fails.
 */
static int handle_escape(
        int inputChar, char* inputBuffer, char** expBuffer, int* expBuffSize)
{
    if (inputChar == KEY_ESCAPE) {
        // Clear input buffer
        inputBuffer[0] = '\0';

        // Reset expression buffer to empty str
        char* tempBuffer = realloc(*expBuffer, sizeof(char) * 1);
        if (tempBuffer == NULL) {
            return EXIT_ERROR;
        }

        *expBuffer = tempBuffer;
        (*expBuffer)[0] = '\0';
        *expBuffSize = 1;

        return EXIT_OK;
    }

    return EXIT_ERROR;
}

/* handle_backspace()
 * -------------------
 * Removes the last character from the input buffer when backspace key
 * (ASCII 127) is pressed. Does nothing if buffer is already empty.
 *
 * inputChar: expected to be backspace key (127).
 * inputBuffer: current numeric buffer to modify.
 *
 * Returns: EXIT_OK when backspace is processed; EXIT_ERROR otherwise.
 */
static int handle_backspace(int inputChar, char* inputBuffer)
{
    if (inputChar == KEY_BACKSPACE) {
        int len = strlen(inputBuffer);

        // Remove last character if buffer is not empty
        if (len > 0) {
            inputBuffer[len - 1] = '\0';
        }

        return EXIT_OK;
    }

    return EXIT_ERROR;
}

/* validate_char()
 * ----------------
 * Validates a character for the current interactive context: it is acceptable
 * if it is a valid digit for cfg.inbase, a valid arithmetic operator, or a
 * recognized special character.
 *
 * inputChar: character to validate.
 * cfg: execution configuration (inbase consulted for digit validity).
 *
 * Returns: EXIT_OK if valid; EXIT_ERROR otherwise.
 */
static int validate_char(char inputChar, Config* cfg)
{
    int validNum = valid_digit_for_base(inputChar, cfg->inbase);
    int validOpr = valid_operand(inputChar);
    int validSpCh = valid_special_char(inputChar);

    if (validNum == EXIT_OK || validOpr == EXIT_OK || validSpCh == EXIT_OK) {
        return EXIT_OK;
    }

    return EXIT_ERROR;
}

/* valid_operand()
 * ----------------
 * Checks whether the character is one of the binary arithmetic operators:
 * '+', '-', '*', '/'.
 *
 * inputChar: character to test.
 *
 * Returns: EXIT_OK if an operator; EXIT_ERROR otherwise.
 */
static int valid_operand(int inputChar)
{
    if (inputChar == '+' || inputChar == '-' || inputChar == '*'
            || inputChar == '/') {
        return EXIT_OK;
    }

    return EXIT_ERROR;
}

/* valid_special_char()
 * ---------------------
 * Checks whether the character is a recognized special key handled by the
 * interactive loop.
 *
 * inputChar: character to test (e.g., '\n', KEY_BACKSPACE, KEY_ESCAPE).
 *
 * Returns: EXIT_OK if recognized; EXIT_ERROR otherwise.
 */
static int valid_special_char(int inputChar)
{
    if (inputChar == '\n' || inputChar == KEY_BACKSPACE
            || inputChar == KEY_ESCAPE) {
        return EXIT_OK;
    }

    return EXIT_ERROR;
}
