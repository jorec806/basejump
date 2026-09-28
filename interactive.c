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

int run_interactive_mode(Config* cfg)
{
    start_session(cfg);

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

static void start_session(Config* cfg)
{
    clear_screen();
    print_greetings();
    print_bases_info(*cfg);
    print_instruction();
    disable_line_buffering();
}

static void end_session(Config* cfg, char** expBuffer)
{
    enable_line_buffering();

    for (int i = 0; i < cfg->historySize; i++) {
        free(cfg->history[i].expression);
        free(cfg->history[i].result);
    }

    free(cfg->history);
    free(*expBuffer);

    print_exit();
}

static int handle_command(char inputChar, Config* cfg, char* commBuff,
        int* commandMode, char* inputBuffer, char** expBuffer,
        int* expBufferSize)
{

    if (inputChar == ':') {
        *commandMode = 1;
        return EXIT_OK;
    }

    if (*commandMode == 1) {
        if (inputChar == '\n') {
            int parsedCommand = parse_command(commBuff, cfg);
            if (parsedCommand == EXIT_OK) {
                inputBuffer[0] = '\0';
                commBuff[0] = '\0';
                *expBuffer[0] = '\0';
                *expBufferSize = 1;
                *commandMode = 0;

            } else {
                *commandMode = 0;
                return EXIT_ERROR;
            }
        } else {

            int len = strlen(commBuff);
            if (len < MAX_INPUT_SIZE) {
                commBuff[len] = inputChar;
                commBuff[len + 1] = '\0';
            }
        }
    }

    return EXIT_OK;
}

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

static int command_history(
        Config* cfg, char* expression, char* result, int inbase)
{

    cfg->historySize++;
    History* temp = realloc(cfg->history, sizeof(History) * cfg->historySize);

    if (temp == NULL) {
        cfg->historySize--;
        return EXIT_ERROR;
    }

    cfg->history = temp;

    int index = cfg->historySize - 1;
    cfg->history[index].expression = malloc(strlen(expression) + 1);
    cfg->history[index].result = malloc(strlen(result) + 1);

    if (cfg->history[index].expression == NULL
            || cfg->history[index].result == NULL) {
        return EXIT_ERROR;
    }

    strcpy(cfg->history[index].expression, expression);
    strcpy(cfg->history[index].result, result);
    cfg->history[index].inbase = inbase;

    return EXIT_OK;
}

static int command_inbase(char* inbase, Config* cfg)
{

    if (strlen(inbase) > 2) {
        return EXIT_ERROR;
    }

    for (int x = 0; inbase[x] != '\0'; x++) {
        if (!(inbase[x] >= '0' && inbase[x] <= '9')) {
            return EXIT_ERROR;
        }
    }

    char* endptr;
    long newBase = strtol(inbase, &endptr, DEFAULT_IN_BASE);

    if (newBase < MIN_BASE || newBase > MAX_BASE) {
        return EXIT_ERROR;
    }

    cfg->inbase = (int)newBase;

    return EXIT_OK;
}

static int handle_printing(int inputChar, Config* cfg, char* inputBuffer,
        char** expBuffer, int* expBuffSize)
{

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

static int handle_operator(
        int inputChar, char* inputBuffer, char** expBuffer, int* expBuffSize)
{

    char* tempBuffer;

    if (strlen(inputBuffer) == 0) {
        inputBuffer[0] = '0';

        inputBuffer[1] = '\0';

        *expBuffSize += 1 + 1;
    } else {
        int len = strlen(inputBuffer);

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

    inputBuffer[0] = '\0';

    return EXIT_OK;
}

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

static int handle_new_line(
        int inputChar, char* inputBuffer, char** expBuffer, int* expBuffSize)
{
    if (inputChar == '\n') {

        if (*expBuffSize >= 2) {

            if (strlen(inputBuffer) == 0) {
                inputBuffer[0] = '0';
                inputBuffer[1] = '\0';
            }

        } else {

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

        return EXIT_OK;
    }

    return EXIT_ERROR;
}

static int handle_escape(
        int inputChar, char* inputBuffer, char** expBuffer, int* expBuffSize)
{
    if (inputChar == KEY_ESCAPE) {

        inputBuffer[0] = '\0';

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

static int handle_backspace(int inputChar, char* inputBuffer)
{
    if (inputChar == KEY_BACKSPACE) {
        int len = strlen(inputBuffer);

        if (len > 0) {
            inputBuffer[len - 1] = '\0';
        }

        return EXIT_OK;
    }

    return EXIT_ERROR;
}

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

static int valid_operand(int inputChar)
{
    if (inputChar == '+' || inputChar == '-' || inputChar == '*'
            || inputChar == '/') {
        return EXIT_OK;
    }

    return EXIT_ERROR;
}

static int valid_special_char(int inputChar)
{
    if (inputChar == '\n' || inputChar == KEY_BACKSPACE
            || inputChar == KEY_ESCAPE) {
        return EXIT_OK;
    }

    return EXIT_ERROR;
}
