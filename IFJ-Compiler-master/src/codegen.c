/**
 * codegen.c - generation of internal representation code
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

#include "codegen.h"

// NOTE this is global variable, it is probably bad practice, but best solution
static const char *instructions[] = {"ADDS", "SUBS", "MULS", "DIVS", "IDIVS"};
static const char *logics[] = {"LTS", "GTS", "EQS", "NOT"};
static int to_include[N_INCLUDES];
static code_generator_t *g_generator;

bool g_string_instr = false;

static codegen_stack_t *codegen_stack_init(bool is_fdata, int size)
{
	codegen_stack_t *new = (codegen_stack_t *)malloc(sizeof(codegen_stack_t));
	if (new == NULL)
		return NULL;

	new->max_size = size;
	new->top = INIT_STACK_TOP;
	if (is_fdata) {
		new->fdata = (if_t *)calloc(sizeof(if_t), size);
		if (new->fdata == NULL) {
			free(new); // new has to be allocated by now
			return NULL;
		}
		return new;
	}
	new->ldata = (int *)calloc(sizeof(int), size);
	if (new->ldata == NULL) {
		free(new); // new has to be allocated by now
		return NULL;
	}
	return new;
}

static bool codegen_stack_push(codegen_stack_t *stack, int new_num,
								node_t *new_node_ptr)
{
	if (stack->top == stack->max_size - 1)
		return false;

	if (stack == g_generator->if_stack) {
		if_t new_if;
		new_if.if_cnt = new_num;
		new_if.branch = 0;

		stack->fdata[++(stack->top)] = new_if;
		g_generator->active_if = &(stack->fdata[stack->top]);
	} else if (stack == g_generator->loop_stack) {
		stack->ldata[++(stack->top)] = new_num;
		g_generator->active_loop = (stack->ldata[stack->top]);
	} else {
		stack->ndata[++(stack->top)] = new_node_ptr;
	}
	return true;
}

static node_t *codegen_stack_pop(codegen_stack_t *stack)
{
	stack->top--;
	if (stack->top == -1)
		return NULL;

	if (stack == g_generator->if_stack)
		g_generator->active_if = &(stack->fdata[stack->top]);
	else if (stack == g_generator->loop_stack)
		g_generator->active_loop = (stack->ldata[stack->top]);
	else
		return stack->ndata[stack->top + 1];
	return NULL;
}

bool codegen_init()
{
	g_generator = malloc(sizeof(code_generator_t));
	if (g_generator == NULL)
		return false;
	
	g_generator->if_stack = codegen_stack_init(true, INIT_STACK_SIZE);
	if (g_generator->if_stack == NULL) {
		free(g_generator); // it should be allocated by now
		return false;
	}

	g_generator->loop_stack = codegen_stack_init(false, INIT_STACK_SIZE);
	if (g_generator->loop_stack == NULL) {
		// all that is going to be freed should be allocated
		free(g_generator->if_stack->fdata);
		free(g_generator->if_stack);
		free(g_generator);
		return false;
	}

	g_generator->if_cnt = 0;
	g_generator->loop_cnt = 0;
	printf(".IFJcode20\n");
	printf("JUMP main\n\n");
	return true;
}

static bool is_newline(char c, char after_c)
{
	return (c == '\\' && after_c == 'n');
}

static bool is_reserved_char(char c)
{
	return ((c > 0 && c <= 32) || (c == 35) || (c == 92));
}

char *str_to_ir_str(char *str)
{
	// WARNING this function is not memory safe
	// TODO rework to make it memory safe
	size_t reserved_cnt = 0;
	for (int i = 0; str[i] != '\0'; i++) {
		if (is_reserved_char(str[i]))
			reserved_cnt++;
	}

	size_t new_str_len = strlen(str) + (reserved_cnt * RESERVED_SIZE) + 1;
	char *new_str = malloc(sizeof(char) * new_str_len);
	if (new_str == NULL)
		return NULL;
	for (size_t i = 0; i < new_str_len; i++)
		new_str[i] = '\0';

	// we need tmp_str because sprintf is undefined for overlaping memory
	char *tmp_str = malloc(sizeof(char) * new_str_len);
	if (tmp_str == NULL) {
		free(new_str);
		return NULL;
	}
	for (size_t i = 0; i < new_str_len; i++)
		tmp_str[i] = '\0';

	for (int j = 0, i = 0; str[i] != '\0'; i++, j++) {
		if (is_newline(str[i], str[i+1])) {
			sprintf(new_str, "%s\\%03d", tmp_str, '\n');
			// we have to jump over the \000
			j += CHAR_CODE_SIZE;
			i++;
		} else if (is_reserved_char(str[i])) {
			sprintf(new_str, "%s\\%03d", tmp_str, str[i]);
			j += CHAR_CODE_SIZE;
		} else {
			new_str[j] = str[i];
		}
		strncpy(tmp_str, new_str, new_str_len);
	}
	free(tmp_str);
	return new_str;
}

void codegen_define(node_t *entry)
{
	printf("DEFVAR %s\n", (entry)->ir);
}

void codegen_new_expr(type_t type)
{
	if (type == INT)
		printf("PUSHS int@0\n");
	else if (type == FLOAT)
		printf("PUSHS float@%a\n", 0.0);
	// TODO add string functionality
}

void codegen_logic(logic_t logic)
{
	if (g_string_instr) {
		codegen_add_include(STRCMP);
		switch (logic) {
		case LESS:
			printf("CALL %%strcmp\nPUSHS int@2\nEQS\n");
			break;
		case MORE:
			printf("CALL %%strcmp\nPUSHS int@1\nEQS\n");
			break;
		case EQUAL:
			printf("CALL %%strcmp\nPUSHS int@0\nEQS\n");
			break;
		case NOT:
			printf("CALL %%strcmp\nPUSHS int@0\nEQS\nNOT\n");
			break;
		case MORE_EQ:
			printf("CALL %%strcmp\nPUSHS int@2\nEQS\nNOT\n");
			break;
		case LESS_EQ:
			printf("CALL %%strcmp\nPUSHS int@1\nEQS\nNOT\n");
			break;
		default:
			break;
		}
		return;
	}
	switch (logic) {
	case LESS:
		printf("%s\n", logics[logic]);
		break;
	case MORE:
		printf("%s\n", logics[logic]);
		break;
	case EQUAL:
		printf("%s\n", logics[logic]);
		break;
	case NOT:
		printf("%s\n", logics[logic]);
		break;
	default:
		break;
	}
}

void codegen_instr(instruction_t instr)
{
	if (g_string_instr) {
		codegen_add_include(STRCMP);
		// only possible instruction for strings is ADD = CONCAT
		printf("POPS LF@TMP2\nPOPS LF@TMP1\n");
		printf("CONCAT LF@TMP1 LF@TMP1 LF@TMP2\nPUSHS LF@TMP1\n");
		return;
	}
	switch (instr) {
	case ADD:
		printf("%s\n", instructions[instr]);
		break;
	case SUB:
		printf("%s\n", instructions[instr]);
		break;
	case MUL:
		printf("%s\n", instructions[instr]);
		break;
	case DIV:
		printf("%s\n", instructions[instr]);
		break;
	case IDIV:
		printf("%s\n", instructions[instr]);
		break;
	case NONE:
		break;
	}
}

void codegen_expr(type_t type, ...)
{
	// WARNING This function is potentially dangerous, there is no way to make
	// it polymorphous and safe at the same time, don't use it with more
	// arguments that three
	va_list args;
	va_start(args, type);
	value_t tmp;
	switch (type) {
	case INT:
		tmp.num = va_arg(args, int64_t);
		printf("PUSHS int@%ld\n", tmp.num);
		break;
	case FLOAT:
		tmp.f_num = va_arg(args, double);
		printf("PUSHS float@%a\n", tmp.f_num);
		break;
	case STRING:
		// TODO Ask Tom where to put it in scanner.c (test11.go)
		tmp.string = str_to_ir_str(va_arg(args, char *));
		printf("PUSHS string@%s\n", tmp.string);
		free(tmp.string);
		break;
	case NODE:
		tmp.node = va_arg(args, node_t *);
		printf("PUSHS %s\n", tmp.node->ir);
		break;
	case STACK:
		break;
	}
	va_end(args);
}

void codegen_write(type_t type, ...)
{
	// WARNING This function is potentially dangerous, there is no way to make
	// it polymorphous and safe at the same time, don't use it with more
	// arguments that three
	va_list args;
	va_start(args, type);
	value_t tmp;
	switch (type) {
	case INT:
		tmp.num = va_arg(args, int64_t);
		printf("WRITE int@%ld\n", tmp.num);
		break;
	case FLOAT:
		tmp.f_num = va_arg(args, double);
		printf("WRITE float@%a\n", tmp.f_num);
		break;
	case STRING:
		tmp.string = str_to_ir_str(va_arg(args, char *));
		printf("WRITE string@%s\n", tmp.string);
		free(tmp.string);
		break;
	case NODE:
		tmp.node = va_arg(args, node_t *);
		printf("WRITE %s\n", tmp.node->ir);
		break;
	case STACK:
		break;
	}
	va_end(args);
}

void codegen_new_func(char *name, int n_params, ...)
{
	printf("\nLABEL %s\n", name);
	printf("CREATEFRAME\n");
	printf("PUSHFRAME\n\n");

	if (!strcmp(name, "main")) {
		printf("DEFVAR GF@TMP\n"); // NOTE I actually dont need it?
		printf("DEFVAR GF@ERR\n");
	}

	printf("DEFVAR LF@TMP1\n");
	printf("DEFVAR LF@TMP2\n\n");

	if (n_params == 0)
		return;

	va_list params;
	va_start(params, n_params);
	node_t *tmp = NULL;
	for (int i = 0; i < n_params; i++) {
		tmp = va_arg(params, node_t *);
		printf("DEFVAR %s\n", tmp->ir);
		printf("POPS %s\n", tmp->ir);
	}
	va_end(params);
	putchar('\n');
}

void codegen_new_param(node_t *param)
{
	printf("DEFVAR %s\n", param->ir);
	printf("POPS %s\n\n", param->ir);
}

void codegen_params(node_t *new_param, bool do_print)
{
	static codegen_stack_t param_stack;
	static bool is_stack_init = false;

	if (!is_stack_init) {
		param_stack.ndata = calloc(sizeof(node_t *), INIT_STACK_SIZE);
		if (param_stack.ndata == NULL)
			return;
		is_stack_init = true;
	}

	if (do_print) {
		node_t *to_be_printed = NULL;
		while (param_stack.top != INIT_STACK_TOP + 1) {
			to_be_printed = codegen_stack_pop(&param_stack);
			codegen_new_param(to_be_printed);
		}
		free(param_stack.ndata);
		is_stack_init = false;
		return;
	}
	codegen_stack_push(&param_stack, 0, new_param);
}

void codegen_assign(node_t *new_assign, bool end)
{
	static codegen_stack_t assign_stack;
	static bool is_stack_init = false;

	if (!is_stack_init) {
		assign_stack.ndata = calloc(sizeof(node_t *), INIT_STACK_SIZE);
		if (assign_stack.ndata == NULL)
			return;
		is_stack_init = true;
	}

	if (end) {
		node_t *to_be_printed = NULL;
		while (assign_stack.top != INIT_STACK_TOP + 1) {
			to_be_printed = codegen_stack_pop(&assign_stack);
			codegen_end_expr(to_be_printed);
		}
		free(assign_stack.ndata);
		is_stack_init = false;
		return;
	}
	codegen_stack_push(&assign_stack, 0, new_assign);
}

void codegen_if_jump()
{
	int next_branch =g_generator->active_if->branch + 1; 
	printf("PUSHS bool@true\n");
	printf("JUMPIFNEQS IF_%d_%d\n", g_generator->active_if->if_cnt, next_branch);
}
void codegen_new_if_statement()
{
	g_generator->if_cnt++;
	codegen_stack_push(g_generator->if_stack, g_generator->if_cnt, NULL);
	printf("\nLABEL IF_%d_0\n", g_generator->active_if->if_cnt);
}

void codegen_if()
{
	printf("LABEL IF_%d_%d\n", g_generator->active_if->if_cnt,
							   ++g_generator->active_if->branch);
}

void codegen_end_if()
{
	printf("JUMP IF_%d_END\n\n", g_generator->active_if->if_cnt);
}

void codegen_end_if_statement()
{
	printf("LABEL IF_%d_%d\n", g_generator->active_if->if_cnt, ++g_generator->active_if->branch );
	printf("LABEL IF_%d_END\n", g_generator->active_if->if_cnt);
	codegen_stack_pop(g_generator->if_stack);
}

void codegen_new_loop()
{
	printf("LABEL LOOP_%d\n", ++g_generator->loop_cnt);
	codegen_stack_push(g_generator->loop_stack, g_generator->loop_cnt, NULL);
}

void codegen_jump_loop()
{
	printf("PUSHS bool@true\n");
	printf("JUMPIFNEQS LOOP_%d_END\n", g_generator->active_loop);
	printf("JUMP LOOP_%d_BODY\n", g_generator->active_loop);
	printf("LABEL LOOP_%d_AFTER\n", g_generator->active_loop);
}

void codegen_after_loop()
{
	printf("JUMP LOOP_%d\n", g_generator->active_loop);
	printf("LABEL LOOP_%d_BODY\n", g_generator->active_loop);
}

void codegen_end_loop()
{
	printf("JUMP LOOP_%d_AFTER\n", g_generator->active_loop);
	printf("LABEL LOOP_%d_END\n", g_generator->active_loop);
	printf("POPFRAME\n");
	codegen_stack_pop(g_generator->loop_stack);
}

// include system, it is appending build in functions on the end of the file
void codegen_add_include(include_t new_include)
{
	static int free_pos = 0;
	for (int i = 0; i < N_INCLUDES; i++)
		// I can include function only once
		if (to_include[i] == (int)new_include)
			return;
	to_include[free_pos] = new_include;
	free_pos++;
}

void codegen_print_includes()
{
	for (int i = 0; i < N_INCLUDES; i++) {
		switch (to_include[i]) {
		case INPUTS:
			printf("LABEL inputs\nCREATEFRAME\nPUSHFRAME\n\n");
			printf("DEFVAR LF@STR\nREAD LF@STR string\n");
			printf("PUSHS LF@STR\n");
			printf("POPFRAME\nRETURN\n\n");
			break;
		case INPUTI:
			printf("LABEL inputi\nCREATEFRAME\nPUSHFRAME\n\n");
			printf("DEFVAR LF@INP\nREAD LF@INP int\n");
			printf("DEFVAR LF@TYP\nTYPE LF@TYP LF@INP\n");
			printf("PUSHS LF@TYP\nPUSHS string@nil\n");
			printf("CALL %%strcmp\nPUSHS int@0\nJUMPIFEQS inputi_err\n");
			printf("PUSHS LF@INP\nPUSHS int@0\n");
			printf("POPFRAME\nRETURN\n");
			printf("LABEL inputi_err\n");
			printf("PUSHS nil@nil\nPUSHS int@1\n");
			printf("POPFRAME\nRETURN\n\n");
			break;
		case INPUTF:
			printf("LABEL inputf\nCREATEFRAME\nPUSHFRAME\n\n");
			printf("DEFVAR LF@INP\nREAD LF@INP float\n");
			printf("DEFVAR LF@TYP\nTYPE LF@TYP LF@INP\n");
			printf("PUSHS LF@TYP\nPUSHS string@nil\n");
			printf("CALL %%strcmp\nPUSHS int@0\nJUMPIFEQS inputf_err\n");
			printf("PUSHS LF@INP\nPUSHS int@0\n");
			printf("POPFRAME\nRETURN\n");
			printf("LABEL inputf_err\n");
			printf("PUSHS nil@nil\nPUSHS int@1\n");
			printf("POPFRAME\nRETURN\n\n");
			break;
		case INT2FLOAT:
			printf("LABEL int2float\nCREATEFRAME\nPUSHFRAME\n\n");
			printf("INT2FLOATS\n");
			printf("POPFRAME\nRETURN\n\n");
			break;
		case FLOAT2INT:
			printf("LABEL float2int\nCREATEFRAME\nPUSHFRAME\n\n");
			printf("FLOAT2INTS\n"); //hodnota je a muze zustat na stacku
			printf("POPFRAME\nRETURN\n\n");
			break;
		case LEN:
			printf("LABEL len\nCREATEFRAME\nPUSHFRAME\n\n");
			printf("DEFVAR LF@T1\n");
			printf("POPS LF@T1\n");
			printf("STRLEN LF@T1 LF@T1\n");
			printf("PUSHS LF@T1\n");
			printf("POPFRAME\nRETURN\n\n");
			break;
		case SUBSTR:
			printf("LABEL substr\nCREATEFRAME\nPUSHFRAME\n\n");
			printf("DEFVAR LF@T1\nDEFVAR LF@T2\nDEFVAR LF@T3\nDEFVAR LF@T4\n\n");
			printf("POPS LF@T3\nPOPS LF@T2\nPOPS LF@T1\n\n");
			// n >= i condition
			printf("PUSHS LF@T3\nPUSHS LF@T2\nEQS\n");
			printf("PUSHS LF@T3\nPUSHS LF@T2\nGTS\nORS\nPUSHS bool@true\n");
			printf("JUMPIFNEQS SUBSTR_ERROR\n\n");
			// i > 0 && n > 0 condition
			printf("PUSHS LF@T2\nPUSHS int@-1\nGTS\nPUSHS LF@T3\nPUSHS int@-1\n");
			printf("GTS\nANDS\nPUSHS bool@true\n");
			printf("JUMPIFNEQS SUBSTR_ERROR\n\n");
			// strlen(str) > i condition
			printf("STRLEN LF@T4 LF@T1\nPUSHS LF@T4\nPUSHS LF@T2\nGTS\n");
			printf("PUSHS bool@true\nJUMPIFNEQS SUBSTR_ERROR\n\n");
			// strlen(str) - i < n condition
			printf("PUSHS LF@T4\nPUSHS LF@T2\nSUBS\nPUSHS LF@T3\nGTS\n");
			printf("PUSHS bool@true\nJUMPIFEQS SUBSTR_MAIN\n");
			printf("PUSHS LF@T4\nPUSHS LF@T2\nSUBS\nPOPS LF@T3\n");
			// main loop
			printf("LABEL SUBSTR_MAIN\n");
			printf("DEFVAR LF@T5\nMOVE LF@T5 string@\n");
			printf("DEFVAR LF@T6\nMOVE LF@T6 string@\n\n");
			printf("LABEL SUBSTR_LOOP\nPUSHS LF@T3\nPUSHS int@0\nGTS\n");
			printf("PUSHS bool@true\nJUMPIFNEQS SUBSTR_END\n\n");
			printf("GETCHAR LF@T5 LF@T1 LF@T2\n");
			printf("ADD LF@T2 LF@T2 int@1\nSUB LF@T3 LF@T3 int@1\n");
			printf("CONCAT LF@T6 LF@T6 LF@T5\n");
			printf("JUMP SUBSTR_LOOP\n\n");
			// return
			printf("LABEL SUBSTR_END\nPUSHS LF@T6\nPUSHS int@0\n");
			printf("POPFRAME\nRETURN\n");
			printf("PUSHS LF@T1\nPUSHS LF@T2\nPOPFRAME\nRETURN\n");
			printf("LABEL SUBSTR_ERROR\nPUSHS string@\nPUSHS int@1\n");
			printf("POPFRAME\nRETURN\n\n");
			break;
		case ORD:
			printf("LABEL ord\nCREATEFRAME\nPUSHFRAME\n");
			printf("DEFVAR LF@T1\nDEFVAR LF@T2\nDEFVAR LF@T3\n\n");
			printf("POPS LF@T2\nPOPS LF@T1\n\n");
			printf("STRLEN LF@T3 LF@T1\n");
			// (strlen - 1) > i && i > -1
			printf("PUSHS LF@T3\nPUSHS int@1\nSUBS\nPUSHS LF@T2\nGTS\n\n");
			printf("PUSHS LF@T2\nPUSHS int@-1\nGTS\nANDS\n");
			printf("PUSHS bool@true\nJUMPIFNEQS ORD_ERROR\n");
			printf("STRI2INT LF@T3 LF@T1 LF@T2\n");
			printf("PUSHS LF@T3\nPUSHS int@0\nPOPFRAME\nRETURN\n");
			printf("LABEL ORD_ERROR\nPUSHS int@0\nPUSHS int@1\n");
			printf("POPFRAME\nRETURN\n\n");
			break;
		case CHR:
			printf("LABEL chr\nCREATEFRAME\nPUSHFRAME\n");
			printf("DEFVAR LF@T1\nDEFVAR LF@T2\n\n");
			printf("POPS LF@T1\n");
			// i > -1 && i < 256
			printf("PUSHS int@-1\nPUSHS LF@T1\nLTS\n");
			printf("PUSHS int@256\nPUSHS LF@T1\nGTS\nANDS\n");
			printf("PUSHS bool@true\nJUMPIFNEQS CHR_ERROR\n");
			printf("INT2CHAR LF@T2 LF@T1\n");
			printf("PUSHS LF@T2\nPUSHS int@0\nPOPFRAME\nRETURN\n");
			printf("LABEL CHR_ERROR\nPUSHS string@\nPUSHS int@1\n");
			printf("POPFRAME\nRETURN\n\n");
			break;
		case STRCMP:
			printf("LABEL %%strcmp\nCREATEFRAME\nPUSHFRAME\n");
			printf("DEFVAR LF@STR1\nDEFVAR LF@STR2\n");
			printf("POPS LF@STR2\nPOPS LF@STR1\n");
			printf("DEFVAR LF@LEN1\nSTRLEN LF@LEN1 LF@STR1\n");
			printf("DEFVAR LF@LEN2\nSTRLEN LF@LEN2 LF@STR2\n");
			printf("DEFVAR LF@LLEN\nMOVE LF@LLEN LF@LEN1\n"); // lower_lenght
			printf("PUSHS LF@LEN1\nPUSHS LF@LEN2\nLTS\n");
			// if they are equal, it is going to store to len1, thus not
			// affecting functionality
			printf("PUSHS bool@true\nJUMPIFEQS strcmp_1_shorter\n");
			printf("MOVE LF@LLEN LF@LEN2\n");
			printf("LABEL strcmp_1_shorter\n");
			printf("DEFVAR LF@T1\nMOVE LF@T1 int@0\n");
			printf("DEFVAR LF@CHR1\nDEFVAR LF@CHR2\n");
			printf("LABEL strcmp_loop\nPUSHS LF@T1\nPUSHS LF@LLEN\nLTS\n");
			printf("PUSHS bool@true\nJUMPIFNEQS strcmp_equal\n");
			printf("STRI2INT LF@CHR1 LF@STR1 LF@T1\n");
			printf("STRI2INT LF@CHR2 LF@STR2 LF@T1\n");
			printf("PUSHS LF@T1\nPUSHS int@1\nADDS\nPOPS LF@T1\n");
			printf("PUSHS LF@CHR1\nPUSHS LF@CHR2\n");
			printf("JUMPIFEQS strcmp_loop\n");
			printf("PUSHS LF@CHR1\nPUSHS LF@CHR2\nLTS\n");
			printf("PUSHS bool@true\nJUMPIFNEQS strcmp_1_greater\n");
			printf("PUSHS LF@CHR1\nPUSHS LF@CHR2\nGTS\n");
			printf("PUSHS bool@true\nJUMPIFNEQS strcmp_2_greater\n");
			// first str is greater
			printf("LABEL strcmp_1_greater\n");
			printf("PUSHS int@1\nPOPFRAME\nRETURN\n");
			// second str is greater
			printf("LABEL strcmp_2_greater\n");
			printf("PUSHS int@2\nPOPFRAME\nRETURN\n");
			// they are to the point of lower lenght equal
			printf("LABEL strcmp_equal\n");
			printf("PUSHS LF@LEN1\nPUSHS LF@LEN2\nEQS\n");
			printf("PUSHS bool@true\nJUMPIFNEQS strcmp_not_equal\n");
			printf("PUSHS int@0\nPOPFRAME\nRETURN\n");
			printf("LABEL strcmp_not_equal\n");
			printf("PUSHS LF@LEN1\nPUSHS LF@LEN2\nGTS\n");
			printf("PUSHS bool@true\nJUMPIFEQS strcmp_1_greater\n");
			printf("PUSHS LF@LEN1\nPUSHS LF@LEN2\nLTS\n");
			printf("PUSHS bool@true\nJUMPIFEQS strcmp_2_greater\n");
			break;
		default: // if we reach 0, there is no more includes
			return;
		}
	}
}

void codegen_dispose()
{
	codegen_print_includes();
	printf("LABEL END\n");
	free(g_generator->if_stack->fdata);
	free(g_generator->if_stack);
	free(g_generator->loop_stack->ldata);
	free(g_generator->loop_stack);
	free(g_generator);
}

#ifdef DEBUG

int main(void)
{
	codegen_init();
	codegen_add_include(STRCMP);
	codegen_dispose();
}
#endif

