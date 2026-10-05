#include <stdio.h>
#include "../parser.h"

int test_tokenizer_simple_expr(void);

/*
 * Casos sugeridos para probar tokenizer (separadores: "+-"):
 *
 * str = " ";          esperado: len 1, [" "]
 * str = "1";          esperado: len 1, ["1"]
 * str = "+";          esperado: len 1, ["+"]
 * str = "-";          esperado: len 1, ["-"]
 * str = "+-";         esperado: len 2, ["+", "-"]
 * str = "--";         esperado: len 2, ["-", "-"]
 * str = "4-";         esperado: len 2, ["4", "-"]
 * str = "4+";         esperado: len 2, ["4", "+"]
 * str = "45734fgdf";  esperado: len 1, ["45734fgdf"]
 * str = "1+2-3";      esperado: len 5, ["1", "+", "2", "-", "3"]
 * str = "+1";         esperado: len 2, ["+", "1"]
 * str = "-1+2";       esperado: len 4, ["-", "1", "+", "2"]
 * str = "1++2";       esperado: len 4, ["1", "+", "+", "2"]
 * str = "1+-2";       esperado: len 4, ["1", "+", "-", "2"]
 * str = "";           esperado: len 0, []
 *
 * Para cada token, comprobar también que value apunte al inicio correcto y
 * que size coincida con la longitud indicada por su contenido.
 */

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
