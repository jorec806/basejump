#include <stdio.h>
#include <stdbool.h>

#include "../parser.h"

int test_tokenizer_space_only(void);
int test_tokenizer_single_token(void);
int test_tokenizer_single_separator(void);
int test_tokenizer_separators_only(void);
int test_tokenizer_repeated_separators(void);
int test_tokenizer_trailing_separator(void);
int test_tokenizer_without_separators(void);
int test_tokenizer_simple_expr(void);
int test_tokenizer_leading_separator(void);
int test_tokenizer_adjacent_different_separators(void);
int test_tokenizer_compound_tokens(void);
int test_tokenizer_empty_string(void);

int main(void)
{
    printf("RUNNING TOKENIZER TESTS\n\n");

    test_tokenizer_space_only();
    test_tokenizer_single_token();
    test_tokenizer_single_separator();
    test_tokenizer_separators_only();
    test_tokenizer_repeated_separators();
    test_tokenizer_trailing_separator();
    test_tokenizer_without_separators();
    test_tokenizer_simple_expr();
    test_tokenizer_leading_separator();
    test_tokenizer_adjacent_different_separators();
    test_tokenizer_compound_tokens();
    test_tokenizer_empty_string();

    return 0;
}

int test_tokenizer_space_only(void)
{
    char* str = " ";
    char* sep = "+-";
    Token* res = NULL;
    size_t len = 0;

    int result = tokenizer(str, sep, &res, &len);

    if (result != 0 || len != 0 || res != NULL){
        printf("Test tokenizer - Space Only : Fail (expected 0 tokens and NULL)\n");
        return -1;
    }

    printf("Test tokenizer - Space Only : Pass\n");
    return 0;
}

int test_tokenizer_single_token(void)
{
    char* str = "1";
    char* sep = "+-";
    Token* res = NULL;
    size_t len = 0;

    int result = tokenizer(str, sep, &res, &len);

    if (res == NULL){
        printf("Test tokenizer - Single Token : Fail (tokens are NULL)\n");
        return -1;
    }

    if (result != 0 || len != 1){
        printf("Test tokenizer - Single Token : Fail (expected 1 token)\n");
        return -1;
    }

    bool t0 = res[0].value[0] == '1' && res[0].size == 1;

    if (t0){
        printf("Test tokenizer - Single Token : Pass\n");
    } else {
        printf("Test tokenizer - Single Token : Fail\n");
        return -1;
    }

    return 0;
}

int test_tokenizer_single_separator(void)
{
    char* sep = "+-";
    char* str = "+";
    Token* res = NULL;
    size_t len = 0;

    int result = tokenizer(str, sep, &res, &len);

    if (res == NULL){
        printf("Test tokenizer - Single Separator : Fail (tokens are NULL)\n");
        return -1;
    }

    if (result != 0 || len != 1){
        printf("Test tokenizer - Single Separator : Fail (expected 1 token)\n");
        return -1;
    }

    bool t0 = res[0].value[0] == '+' && res[0].size == 1;

    if (!t0){
        printf("Test tokenizer - Single Separator : Fail\n");
        return -1;
    }

    str = "-";
    res = NULL;
    len = 0;
    result = tokenizer(str, sep, &res, &len);

    if (res == NULL){
        printf("Test tokenizer - Single Separator : Fail (tokens are NULL)\n");
        return -1;
    }

    if (result != 0 || len != 1){
        printf("Test tokenizer - Single Separator : Fail (expected 1 token)\n");
        return -1;
    }

    bool t1 = res[0].value[0] == '-' && res[0].size == 1;

    if (t1){
        printf("Test tokenizer - Single Separator : Pass\n");
    } else {
        printf("Test tokenizer - Single Separator : Fail\n");
        return -1;
    }

    return 0;
}

int test_tokenizer_separators_only(void)
{
    char* str = "+-";
    char* sep = "+-";
    Token* res = NULL;
    size_t len = 0;

    int result = tokenizer(str, sep, &res, &len);

    if (res == NULL){
        printf("Test tokenizer - Separators Only : Fail (tokens are NULL)\n");
        return -1;
    }

    if (result != 0 || len != 2){
        printf("Test tokenizer - Separators Only : Fail (expected 2 tokens)\n");
        return -1;
    }

    bool t0 = res[0].value[0] == '+' && res[0].size == 1;
    bool t1 = res[1].value[0] == '-' && res[1].size == 1;

    if (t0 && t1){
        printf("Test tokenizer - Separators Only : Pass\n");
    } else {
        printf("Test tokenizer - Separators Only : Fail\n");
        return -1;
    }

    return 0;
}

int test_tokenizer_repeated_separators(void)
{
    char* str = "1++2--3";
    char* sep = "+-";
    Token* res = NULL;
    size_t len = 0;

    int result = tokenizer(str, sep, &res, &len);

    if (res == NULL){
        printf("Test tokenizer - Repeated Separators : Fail (tokens are NULL)\n");
        return -1;
    }

    if (result != 0 || len != 7){
        printf("Test tokenizer - Repeated Separators : Fail (expected 7 tokens)\n");
        return -1;
    }

    bool t0 = res[0].value[0] == '1' && res[0].size == 1;
    bool t1 = res[1].value[0] == '+' && res[1].size == 1;
    bool t2 = res[2].value[0] == '+' && res[2].size == 1;
    bool t3 = res[3].value[0] == '2' && res[3].size == 1;
    bool t4 = res[4].value[0] == '-' && res[4].size == 1;
    bool t5 = res[5].value[0] == '-' && res[5].size == 1;
    bool t6 = res[6].value[0] == '3' && res[6].size == 1;

    if (t0 && t1 && t2 && t3 && t4 && t5 && t6){
        str = "++--";
        res = NULL;
        len = 0;
        result = tokenizer(str, sep, &res, &len);

        if (res == NULL){
            printf("Test tokenizer - Repeated Separators : Fail (tokens are NULL)\n");
            return -1;
        }

        if (result != 0 || len != 4){
            printf("Test tokenizer - Repeated Separators : Fail (expected 4 tokens)\n");
            return -1;
        }

        bool t7 = res[0].value[0] == '+' && res[0].size == 1;
        bool t8 = res[1].value[0] == '+' && res[1].size == 1;
        bool t9 = res[2].value[0] == '-' && res[2].size == 1;
        bool t10 = res[3].value[0] == '-' && res[3].size == 1;

        if (t7 && t8 && t9 && t10){
            printf("Test tokenizer - Repeated Separators : Pass\n");
        } else {
            printf("Test tokenizer - Repeated Separators : Fail\n");
            return -1;
        }
    } else {
        printf("Test tokenizer - Repeated Separators : Fail\n");
        return -1;
    }

    return 0;
}

int test_tokenizer_trailing_separator(void)
{
    char* str = "4-";
    char* sep = "+-";
    Token* res = NULL;
    size_t len = 0;

    int result = tokenizer(str, sep, &res, &len);

    if (res == NULL){
        printf("Test tokenizer - Trailing Separator : Fail (tokens are NULL)\n");
        return -1;
    }

    if (result != 0 || len != 2){
        printf("Test tokenizer - Trailing Separator : Fail (expected 2 tokens)\n");
        return -1;
    }

    bool t0 = res[0].value[0] == '4' && res[0].size == 1;
    bool t1 = res[1].value[0] == '-' && res[1].size == 1;

    if (!t0 || !t1){
        printf("Test tokenizer - Trailing Separator : Fail\n");
        return -1;
    }

    str = "4+";
    res = NULL;
    len = 0;
    result = tokenizer(str, sep, &res, &len);

    if (res == NULL){
        printf("Test tokenizer - Trailing Separator : Fail (tokens are NULL)\n");
        return -1;
    }

    if (result != 0 || len != 2){
        printf("Test tokenizer - Trailing Separator : Fail (expected 2 tokens)\n");
        return -1;
    }

    bool t2 = res[0].value[0] == '4' && res[0].size == 1;
    bool t3 = res[1].value[0] == '+' && res[1].size == 1;

    if (t2 && t3){
        printf("Test tokenizer - Trailing Separator : Pass\n");
    } else {
        printf("Test tokenizer - Trailing Separator : Fail\n");
        return -1;
    }

    return 0;
}

int test_tokenizer_without_separators(void)
{
    char* str = "45734fgdf";
    char* sep = "+-";
    Token* res = NULL;
    size_t len = 0;

    int result = tokenizer(str, sep, &res, &len);

    if (res == NULL){
        printf("Test tokenizer - Without Separators : Fail (tokens are NULL)\n");
        return -1;
    }

    if (result != 0 || len != 1){
        printf("Test tokenizer - Without Separators : Fail (expected 1 token)\n");
        return -1;
    }

    bool t0 = res[0].value == str && res[0].size == 9;

    if (t0){
        printf("Test tokenizer - Without Separators : Pass\n");
    } else {
        printf("Test tokenizer - Without Separators : Fail\n");
        return -1;
    }

    return 0;
}

int test_tokenizer_simple_expr(void)
{
    char* str = "1+2-3";
    char* sep = "+-";
    Token* res = NULL;
    size_t len = 0;

    int result = tokenizer(str, sep, &res, &len);

    if (res == NULL){
        printf("Test tokenizer - Simple Expression : Fail (tokens are NULL)\n");
        return -1;
    }

    if (result != 0 || len != 5){
        printf("Test tokenizer - Simple Expression : Fail (expected 5 tokens)\n");
        return -1;
    }

    bool t0 = res[0].value[0] == '1' && res[0].size == 1;
    bool t1 = res[1].value[0] == '+' && res[1].size == 1;
    bool t2 = res[2].value[0] == '2' && res[2].size == 1;
    bool t3 = res[3].value[0] == '-' && res[3].size == 1;
    bool t4 = res[4].value[0] == '3' && res[4].size == 1;

    if (t0 && t1 && t2 && t3 && t4){
        printf("Test tokenizer - Simple Expression : Pass\n");
    } else {
        printf("Test tokenizer - Simple Expression : Fail\n");
        return -1;
    }

    return 0;
}

int test_tokenizer_leading_separator(void)
{
    char* str = "+1";
    char* sep = "+-";
    Token* res = NULL;
    size_t len = 0;

    int result = tokenizer(str, sep, &res, &len);

    if (res == NULL){
        printf("Test tokenizer - Leading Separator : Fail (tokens are NULL)\n");
        return -1;
    }

    if (result != 0 || len != 2){
        printf("Test tokenizer - Leading Separator : Fail (expected 2 tokens)\n");
        return -1;
    }

    bool t0 = res[0].value[0] == '+' && res[0].size == 1;
    bool t1 = res[1].value[0] == '1' && res[1].size == 1;

    if (!t0 || !t1){
        printf("Test tokenizer - Leading Separator : Fail\n");
        return -1;
    }

    str = "-1";
    res = NULL;
    len = 0;
    result = tokenizer(str, sep, &res, &len);

    if (res == NULL){
        printf("Test tokenizer - Leading Separator : Fail (tokens are NULL)\n");
        return -1;
    }

    if (result != 0 || len != 2){
        printf("Test tokenizer - Leading Separator : Fail (expected 2 tokens)\n");
        return -1;
    }

    bool t2 = res[0].value[0] == '-' && res[0].size == 1;
    bool t3 = res[1].value[0] == '1' && res[1].size == 1;

    if (t2 && t3){
        printf("Test tokenizer - Leading Separator : Pass\n");
    } else {
        printf("Test tokenizer - Leading Separator : Fail\n");
        return -1;
    }

    return 0;
}

int test_tokenizer_adjacent_different_separators(void)
{
    char* str = "1+-2";
    char* sep = "+-";
    Token* res = NULL;
    size_t len = 0;

    int result = tokenizer(str, sep, &res, &len);

    if (res == NULL){
        printf("Test tokenizer - Adjacent Different Separators : Fail (tokens are NULL)\n");
        return -1;
    }

    if (result != 0 || len != 4){
        printf("Test tokenizer - Adjacent Different Separators : Fail (expected 4 tokens)\n");
        return -1;
    }

    bool t0 = res[0].value[0] == '1' && res[0].size == 1;
    bool t1 = res[1].value[0] == '+' && res[1].size == 1;
    bool t2 = res[2].value[0] == '-' && res[2].size == 1;
    bool t3 = res[3].value[0] == '2' && res[3].size == 1;

    if (t0 && t1 && t2 && t3){
        printf("Test tokenizer - Adjacent Different Separators : Pass\n");
    } else {
        printf("Test tokenizer - Adjacent Different Separators : Fail\n");
        return -1;
    }

    return 0;
}

int test_tokenizer_compound_tokens(void)
{
    char* str = "45+89-105";
    char* sep = "+-";
    Token* res = NULL;
    size_t len = 0;

    int result = tokenizer(str, sep, &res, &len);

    if (res == NULL){
        printf("Test tokenizer - Compound Tokens : Fail (tokens are NULL)\n");
        return -1;
    }

    if (result != 0 || len != 5){
        printf("Test tokenizer - Compound Tokens : Fail (expected 5 tokens)\n");
        return -1;
    }

    bool t0 = res[0].value == str && res[0].size == 2;
    bool t1 = res[1].value[0] == '+' && res[1].size == 1;
    bool t2 = res[2].value == str + 3 && res[2].size == 2;
    bool t3 = res[3].value[0] == '-' && res[3].size == 1;
    bool t4 = res[4].value == str + 6 && res[4].size == 3;

    if (t0 && t1 && t2 && t3 && t4){
        printf("Test tokenizer - Compound Tokens : Pass\n");
    } else {
        printf("Test tokenizer - Compound Tokens : Fail\n");
        return -1;
    }

    return 0;
}

int test_tokenizer_empty_string(void)
{
    char* str = "";
    char* sep = "+-";
    Token* res = NULL;
    size_t len = 0;

    int result = tokenizer(str, sep, &res, &len);

    if (result == 0 && len == 0 && res == NULL){
        printf("Test tokenizer - Empty String : Pass\n");
    } else {
        printf("Test tokenizer - Empty String : Fail\n");
        return -1;
    }

    return 0;
}
