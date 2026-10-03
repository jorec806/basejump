#include <stdio.h>
#include "../parser.h"

int test_tokenizer_simple_expr(void);

int main(void){

    printf("RUNING TEST\n\n");
    // Tokenizer test
    test_tokenizer_simple_expr();
}

int test_tokenizer_simple_expr(void)
{
    char* str = "1+2-3";
    char* sep = "+-";
    Token* res = NULL;
    size_t len = 0;

    int result = tokenizer (str, sep, &res, &len);

    if (result == EXIT_OK && len == 5){
        printf("Test tokenizer - Simple Expression : Pass\n");
    } else {
        printf("Test tokenizer - Simple Expression : Fail\n");

        if(len != 5){
            printf("test len = 5 - function len = %zu\n", len);
        }
    }

    return 0;
}
