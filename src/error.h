/**
 * error.h - Error codes and printing functions
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
 *   xkrizd03 - Daniel Kříž
 */

#ifndef ERROR_H
#define ERROR_H

#define ERROR(msg) fprintf(stderr, "ERROR: %s\n", (msg));

// fault may be replaced with error
enum error_codes { 
	OK,
	LEXICAL_ERROR,
	SYNTAX_ERROR,
	DEFINITION_ERROR,
	VAR_TYPE_ERROR,
	TYPE_COMPABILITY_ERROR,
	PARAM_CNT_ERROR,
	RETURN_ERROR=6,
	SEMANTIC_ERROR,
	ZERO_DIV_ERROR=9,
	ERROR_CODE=99,
};

#endif // ERROR_H
