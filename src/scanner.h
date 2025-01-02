/**
 * scanner.h - Header for Lexical analysis
 * 
 * IAL, IFJ project 2020
 * Team:
 *   xkriva29 - Tomáš Křivánek
 *   xkrizd03 - Daniel Kříž
 *   xhomol27 - Ján Homola
 *   xgorca00 - Damián Gorčák
 *
 * Maintainers:
 *   xkriva29 - Tomáš Křivánek
 */

#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>

#define LEX_ERROR 1
#define DEFAULT_SIZE 20

extern FILE *source_file;
extern FILE* print_file;
extern int line_counter;

typedef enum lex_modes {
    MODE_NUMBER,
    MODE_STRING,
    MODE_IDENTIFIER_KEYWORD,
    MODE_NEW_LINE,
    MODE_ASIGN,
    MODE_PLUS,
    MODE_MULTIPLY,
    MODE_MINUS,
    MODE_DIVIDE,
    MODE_GREATER,
    MODE_LESS,
    MODE_NOT_EQUAL,
    MODE_DEFINE,
    MODE_BRACKET,
    MODE_SEMICOLON,
    MODE_COLON,
    MODE_CR,
    MODE_EOF,
    MODE_UNDEFINED,
} Mode;




typedef enum token_type {
	TOKEN_INT,
    TOKEN_INT_EXP,
	TOKEN_FLOAT,
    TOKEN_FLOAT_EXP,
    TOKEN_KEYWORD_INT,
    TOKEN_KEYWORD_FLOAT,
    TOKEN_KEYWORD_STRING,
	TOKEN_STRING,
	TOKEN_VAR, // variable id
    TOKEN_VAR_UNDERSCORE,
	TOKEN_FUNC, // function id
    TOKEN_MAIN_FUNC,
    TOKEN_PACKAGE,
    TOKEN_RETURN,
    TOKEN_PRINT,
    TOKEN_VAR_FUNC,
    TOKEN_INPUT_INT,
    TOKEN_INPUT_FLOAT,
    TOKEN_INPUT_STRING,
    TOKEN_INT2FLOAT,
    TOKEN_FLOAT2INT,
    TOKEN_LEN,
    TOKEN_SUBSTR,
    TOKEN_ORD,
    TOKEN_CHR,
	TOKEN_FOR,
	TOKEN_IF,
	TOKEN_WHILE,
	TOKEN_ELSE,
	TOKEN_IF_ELSE,
    TOKEN_NEW_LINE,
    TOKEN_SEMICOLON,
    TOKEN_COLON,
	TOKEN_PLUS,
    TOKEN_MULTIPLY,
	TOKEN_MINUS,
    TOKEN_DIVISION,
	TOKEN_ASSIGN, // =
	TOKEN_DEFINE, // :=
	TOKEN_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_LEFT_ROUND,
    TOKEN_RIGHT_ROUND,
    TOKEN_LEFT_CURLY,
    TOKEN_RIGHT_CURLY, 
    TOKEN_EOF,
    TOKEN_UNDEFINED,
    TOKEN_DOLLAR,
    TOKEN_SHIFT, //DOROBIL SI POVEDDZ CHLAPOM
    TOKEN_NON_TERM,
} token_t;


typedef struct dynamic_string_st {
    char* string;
    int alocated;
    int actual_length;
} dyn_string_t;

typedef union dataTok {
    int64_t integer;
    double dbl;
    dyn_string_t string;
} Data;

typedef struct Token_st {
    enum token_type type;
    int* pointer;
    Data data;
} Token;

typedef struct dyn_str_list_item{
    Token* list_token;
    struct dyn_str_list_item *next;
} Dyn_str_item;

typedef struct dyn_str_list {
    Dyn_str_item *first;
} Dyn_str_list;



extern Dyn_str_list *list_of_dyn_strings;


void close_files();

/**
 * @brief The most important function for scanner (Gets next token from source_file)
 * @return next Token in source_file
 */
Token getToken ();
/**
 * @brief Printing function for debugging purposes -> Prints informations about tokens
 * @param newToken Token, which we need to be printed
 * @param print_file File, in which we want to print
 */
void print_token(Token newToken,FILE* print_file);
/**
 * @brief Set the variable source_file, which we need in getToken
 * @param file_pointer Pointer to file (which we want to parse)
 */
void set_source_file(FILE* file_pointer);
/**
 * @brief Returns next character in source_file (without changing position of pointer)
 * @param pfile Pointer to our source_file
 * @return next character in source file
 */
char lookAhead(FILE* pfile);



/**
 * @brief Decides if entered word is a keyword
 * @param word pointer to first char of specific word, which we want to know, if it is keyword
 * @return True if word is a keyword, otherwise false
 */
bool isKeyword (char* word);

/**
 * @brief Decides if entered character is one of letter/underscore/number ('0'-'9','a'-'z','A'-'Z','_')
 * @param c character, about what we need to know if it is letter/underscore/number
 * @return True if char c is one of letter/underscore/number ('0'-'9','a'-'z','A'-'Z','_')
 */
bool is_letter_or_underscore_or_number(char c);
/**
 * @brief Resets the inputed list of characters to \0
 * @param list Which we need to reset
 * @param length length of list
 */
void clearList(char* list, int length);
/**
 * @brief Compares string1 and string2
 * @param string1 Pointer to first character of word 1
 * @param string2 Pointer to first character of word 2
 * @return True if both strings are the same, false otherwise
 */
bool myStringCMP(char *string1, char *string2);
/**
 * @brief Processes the backslash expressions in string
 * @param word Pointer to list of chars, where i am saving the string
 * @return True if something went wrong otherwise false
 */
bool process_backslash(dyn_string_t *word);


/**
 * @brief Initializes dynamic string
 * @return Returns initialized dynamic string
 */
dyn_string_t str_init();
/**
 * @brief Adds character to the end of dynamic string
 * @param string Pointer to dynamic string
 * @param c Character which we want to add on the end of string
 */
void str_append(dyn_string_t *string,char c);

/**
 * @brief Frees allocated memory for dynamic string
 * @param string Pointer to dynamic string
 */
void str_free(dyn_string_t *string);
/**
 * @brief copies a dynamic string to another one
 * @param string2 Location where we want to copy
 * @param string1 Location from where we want to copy
 */
void dyn_str_cpy(dyn_string_t *string2,dyn_string_t *string1);

/**
 * @brief Initialize linked list of dynamic strings 
 * @param list pointer to list of dynamic strings, where we want to allocate our list
 */
void dyn_str_list_init();
/**
 * @brief Inserts a dynamic string to the beggining of list 
 * @param list pointer to list of dynamic strings
 * @param string Dynamic string, which we want to add
 */
void insert_to_dyn_list(Dyn_str_list *list,Token* insert_token);
/**
 * @brief Removes list of dynamic strings
 * @param list Pointer to allocated list of dynamic strings, which we want to delete
 */
void dispose_dyn_str_list(Dyn_str_list *list);

#endif //SCANNER_H
