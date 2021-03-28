/**
 * expression.c - Header for semantical analysis
 * 
 * IAL, IFJ project 2020
 * Team:
 *   xkriva29 - Tomáš Křivánek
 *   xkrizd03 - Daniel Kříž
 *   xhomol27 - Ján Homola
 *   xgorca00 - Damián Gorčák
 *
 * Maintainers:
 *   xgorca00 - Damián Gorčák
 */

#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "parser.h"
#include "scanner.h"
#include "symtable.h"
#include "error.h"
#include "codegen.h"


//Token actual_token ;

//bool is_bool;
extern int where;
extern bool token_was_war;

typedef struct  list 
{
    Token symbol ;
	enum token_type type;
	int i_value;
	double d_value;
    struct list *next ;
	char string[256];
} list;
typedef list *stack;

typedef enum
{
	L,    // < less
	E,    // = EQUAL
	M,    // > more
	ERR,  //  ERROR
    DONE    // $ == $
} Prec_table_symbol_enum;

typedef enum
{
	NON_SYMBOL_IN_EXPRESSION = -1,    	// non symbol in terminal
	PLUS_MINUS,    						// +-
	MULT_DIV,   						// */
	RELATIONS,  						//  < > != == <= >=
    LEFT_BR,    						// (
	RIGHT_BR,   						// )
	IDENTIFIER, 						// i
	DOLAR								// $
} Prec_return;

typedef enum
{
	BOOL_FALSE = 1,
	BOOL_TRUE ,
	DECLARATION,
	ASSIGN,
	IN_MULTIPE,
	IN_PRINT	
} Where_am_i;


bool push( list **stack, Token *sym,Token skuska);

/**
 * @brief it pushes on an element on the top of the stack
 * @param stack is stack where we want to push elements
 * @param sym it is elemen which we want to be pushed
 * @return true if push was done succesfully
 * @return false if push was done unsuccesfully
 */

Token pop(stack *mystack);

/**
 * @brief it pops the top element from the stack
 * @param stack is stack where we want to pop element
 * @return top element which was popped
 */

void print(stack *mystack);

/**
 * @brief it prints all elements which are in stack
 * @param stack is stack from which we want print all elements
 * @return true if push was done succesfully
 * @return false if push was done unsuccesfully
 */

Token top_terminal_in_stack(stack *mystack);

/**
 * @brief it is searching for fist element from the top which is terminal
 * @param stack is stack where we want serch top terminal
 * @return fist element from the top which is terminal
 */

void insert_after_last(stack *mystack);

/**
 * @brief it is inserting symblol "<" after the first terminal element from the top in stack
 * @param stack is stack where we want serch top terminal
 * @param symbol is symbol "<" which we want insert
 */


int semantic(Token first_nonterm, Token operand, Token second_nonterm,int counter,Token *final_non,stack *mystack, stab_dict_t dict);

/**
 * @brief it tests semantic in expression by the rules
 * @param first_nonterm is first nonterm from rule
 * @param operand is operant from rule
 * @param operand seconf_nonterm is first nonterm from rule
 * @param counter is count how many nonterms we get -> we can get 1 - when we have only one nonterm  or 3 when we get whole rule
 */

int reduce(stack *mystack, stab_dict_t dict);

/**
 * @brief it is reducing expression and call semantic function
 * @param stack is stack where we want serch top terminal
 * @pre it is popping out from stack whole rule we tested + symbol shift
 * @post it pushes nonterminal to the stack after processing whole rule
 */

int expression(stab_dict_t dict, root_t scope_table, Token where_store_type);

#endif //EXPRESSION_H;
