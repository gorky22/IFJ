/**
 * codegen.h - header for generation of internal representation code
 * 
 * IAL, IFJ project 2020
 * Team:
 *   xkriva29 - Tomáš Křivánek
 *   xkrizd03 - Daniel Kříž
 *   xhomol27 - Ján Homola
 *   xgorca00 - Damián Gorčák
 *
 * Maintainers:
 *   xkrizd03 - Daniel Kříž
 */

#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include "symtable.h"

#define STACK_ERROR NULL
#define INIT_STACK_TOP -1
#define INIT_STACK_SIZE 100
#define CHAR_CODE_SIZE 3
#define RESERVED_SIZE 4
#define N_INCLUDES 10
#define TOKEN_TO_INCLUDE(token) token - 15

extern bool g_string_instr;

typedef struct {
	int if_cnt;
	int branch;
} if_t;

typedef struct {
	int max_size;
	int top;
	union {
		int *ldata;
		if_t *fdata;
		node_t **ndata;
	};
} codegen_stack_t;

typedef struct {
	codegen_stack_t *if_stack;
	codegen_stack_t *loop_stack;
	int if_cnt;
	int loop_cnt;
	int active_loop;
	if_t *active_if;
} code_generator_t;

typedef enum instruction {
	ADD,
	SUB,
	MUL,
	DIV,
	IDIV,
	NONE
} instruction_t;

typedef enum type {
	INT,
	FLOAT,
	STRING,
	NODE,
	STACK
} type_t;

typedef enum {
	LESS,
	MORE,
	EQUAL,
	NOT,
	OR,
	MORE_EQ,
	LESS_EQ,
} logic_t;

typedef union {
	int64_t num;
	double f_num;
	char *string;
	node_t *node;
} value_t;

typedef enum {
	INPUTI = 1, // static global arrays initialize to 0
	INPUTF,
	INPUTS,
	INT2FLOAT,
	FLOAT2INT,
	LEN,
	SUBSTR,
	ORD,
	CHR,
	STRCMP
} include_t;

#define codegen_end_expr(entry) \
{ \
	printf("POPS %s\n\n", (entry)->ir);\
}
#define codegen_ret_func() {printf("\nPOPFRAME\nRETURN\n");}
#define codegen_end_func() {printf("POPFRAME\n");}
#define codegen_call_func(name) {printf("CALL %s\n", name);}

// Codegen Usage
void codegen_after_loop();
void codegen_add_include(include_t new_include);
bool codegen_init();
void codegen_dispose();
char *str_to_ir_str(char *str); // NOTE this may be static in future
// Variable definition
void codegen_define(node_t *entry);
// Expression generation
void codegen_new_expr(type_t type);
void codegen_expr(type_t type, ...);
void codegen_instr(instruction_t instr);
void codegen_logic(logic_t logic);
void codegen_params(node_t *new_param, bool do_print);
void codegen_assign(node_t *new_assign, bool end);
// Build-in functions
void codegen_write(type_t type, ...);
// TODO: codegen_int2float() codegen_float2int() codegen_int2char()
// codegen_str2int()
// TODO: string related functions, to move, concatenate, re-write, strlen
// Functions related to make internal represenation of functions
void codegen_new_func(char *name, int n_params, ...);
// Logical conditions related functions
void codegen_cond(logic_t logic, ...);
// If statement related functions
void codegen_if_jump();
void codegen_new_if_statement();
void codegen_if();
void codegen_end_if();
void codegen_end_if_statement();
// Loop related functions
void codegen_new_loop();
void codegen_jump_loop();
void codegen_end_loop();

#endif // CODEGEN_H
