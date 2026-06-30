#include <stdio.h>
#include "parser.h"

/* print_greetings()
 *  ------------------
 * Prints the program greeting banner.
 *
 * Returns: nothing
 */
void print_greetings()
{
    printf("Welcome to basejump!\n");
    printf("This program was written by s4918633.\n");
}

void print_instruction()
{
    printf("Please enter your numbers and expressions to be converted"
           " and evaluated.\n");
}

/* print_bases_info()
 * -------------------
 * Prints the input base and the list of output bases.
 *
 * config: execution configuration (inbase, obases[0..numBases-1], numBases).
 *
 * Returns: nothing
 */
void print_bases_info(Config config)
{
    printf("Input base: %d\n", config.inbase);
    printf("Output bases: ");

    for (int x = 0; x < (int)config.numBases; x++) {
        if (x) {
            printf(", %d", config.obases[x]);
        } else {
            printf("%d", config.obases[x]);
        }
    }
    printf("\n");
}

void print_exit()
{
    printf("Thank you for using basejump.\n");
}
