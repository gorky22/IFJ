/**
 * parser.h - Header for syntactical analysis
 * 
 * IAL, IFJ project 2020
 * Team:
 *   xkriva29 - Tomáš Křivánek
 *   xkrizd03 - Daniel Kříž
 *   xhomol27 - Ján Homola
 *   xgorca00 - Damián Gorčák
 *
 * Maintainers:
 *   xhomol27 - Ján Homola
 */

#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>
#include <stdbool.h>

#include "scanner.h"
#include "error.h"
#include "symtable.h"
#include "scanner.h"
#include "expression.h"


#define SYNTAX_OK 0
//#define SYNTAX_ERROR 2

extern Token actual_token;
extern char key[256];

extern char return_typ[256];
extern int index_return;



Token get_new_token (bool skip_new_line);

int function(stab_dict_t dict, root_t scope_table);
int expr(stab_dict_t dict, root_t scope_table);
int expr_n(stab_dict_t dict, root_t scope_table);
int scope(stab_dict_t dict, root_t scope_table);
int return_type(stab_dict_t dict, root_t scope_table);
int type(stab_dict_t dict, root_t scope_table);
int type_without_eps(stab_dict_t dict, root_t scope_table);
int type_n(stab_dict_t dict, root_t scope_table);
int statement(stab_dict_t dict, root_t scope_table);
int dec_and_def(stab_dict_t dict, root_t scope_table);
int for_init(stab_dict_t dict, root_t scope_table);
int assign(stab_dict_t dict, root_t scope_table);
int next_id(stab_dict_t dict, root_t scope_table);
int param(stab_dict_t dict, root_t scope_table);
int param_n(stab_dict_t dict, root_t scope_table);
int arg(stab_dict_t dict, root_t scope_table, node_t* tmp_func_node);
int arg_n(stab_dict_t dict, root_t scope_table, node_t* tmp_func_node);
int value(stab_dict_t dict, node_t* tmp_func_node);
int id_func();
int prog(stab_dict_t dict);
int prolog(stab_dict_t dict);

int parser();

//int expression();




// moje pomocne premenne
extern FILE *my_print_file;

#endif // PARSER_H
