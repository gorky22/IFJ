/**
 * main.c - Compiler
 * 
 * IAL, IFJ project 2020
 *
 * Team:
 *   xkriva29 - Tomáš Křivánek
 *   xkrizd03 - Daniel Kříž
 *   xhomol27 - Ján Homola
 *   xgorca00 - Damián Gorčák
 *
 * Maintainers:
 *   xkriva29 - Tomáš Křivánek
 *   xkrizd03 - Daniel Kříž
 *   xhomol27 - Ján Homola
 *   xgorca00 - Damián Gorčák
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "scanner.h"
#include "parser.h"
//#include "expression.h"


int main (){
    FILE* my_file = fopen("tests/test4.go","r");
    //FILE* my_file = stdin;
    set_source_file(my_file);

    if(source_file == NULL)
        exit(1);
    
    codegen_init();
    int result = parser();
    codegen_dispose();

    close_files();
    return result;
}
