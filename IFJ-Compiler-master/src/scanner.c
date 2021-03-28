/**
 * scanner.c - Lexical Analysis of code
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

#include "scanner.h"

FILE *source_file;
FILE* print_file;
int line_counter = 1;
Dyn_str_list *list_of_dyn_strings;
static Token ErrorToken = {.type=TOKEN_UNDEFINED};

char lookAhead (FILE* pfile) {
    char c = fgetc(pfile);
	if(c == EOF)
		return EOF;
    fseek(pfile,-1,SEEK_CUR);
    return c;
}


void set_source_file(FILE* file_pointer){
	source_file = file_pointer;
}

void close_files(){
	fclose(source_file);
}

dyn_string_t str_init() {
    dyn_string_t new_string;
    new_string.string = malloc(DEFAULT_SIZE*sizeof(char));

    for(int i = 0; i < DEFAULT_SIZE; i++)
        new_string.string[i] = '\0';

    new_string.alocated = DEFAULT_SIZE;
    new_string.actual_length = 0;
    return new_string;
}

void str_append(dyn_string_t *string,char c){
    if(string->string == NULL)
        return;


    if(string->actual_length + 1 < string->alocated)
        string->string[string->actual_length++] = c;
    else{
        int new_size = string->alocated + DEFAULT_SIZE;
        string->string = realloc(string->string,new_size*sizeof(char));
        string->alocated = new_size;
        string->string[string->actual_length++] = c;
    }
    string->string[string->actual_length] = '\0';
}

void str_free(dyn_string_t *string){
    if(string->string == NULL)
        return;
    else {
        free(string->string);
        string->alocated = 0;
        string->actual_length = 0;
    }
}

void dyn_str_cpy(dyn_string_t *string2,dyn_string_t *string1){
	if(string1->string == NULL)
		return;
	(*string2) = str_init();
	
	string2->string = realloc(string2->string,(string1->actual_length+1)*sizeof(char));
	strncpy(string2->string,string1->string,string1->actual_length);
	string2->actual_length = string1->actual_length;
	string2->string[string1->actual_length] = '\0';

}


void dyn_str_list_init(){
	list_of_dyn_strings = malloc(sizeof(Dyn_str_list));
	if(list_of_dyn_strings == NULL)
		return;
	list_of_dyn_strings->first = NULL;
}
void insert_to_dyn_list(Dyn_str_list *list,Token* insert_token){
	Dyn_str_item* new_item = malloc(sizeof(Dyn_str_item));
	if(new_item == NULL)
		return;

	new_item->list_token = insert_token;
	new_item->next = list->first;
	list->first = new_item;
}
void dispose_dyn_str_list(Dyn_str_list *list){
	Dyn_str_item* tmp;
	bool first = true;
	for(Dyn_str_item* item = list->first; item != NULL; item = tmp){
		if(first){
			list->first = NULL;
			first = false;
		}
		tmp = item->next;
		str_free(&(item->list_token->data.string));
		free(item->list_token);
		free(item);
	}
	free(list);
}

static bool process_line_comment(char* first_char){
	while(true) {
				*first_char = fgetc(source_file);
				if(lookAhead(source_file) == EOF){
					return false;
				}
				if(lookAhead(source_file) == '\n'){
					*first_char = fgetc(source_file);
					break;
				}
		}
	return true;
}

static bool process_multiple_line_comment(char* first_char){
	*first_char = fgetc(source_file); //skipping the '*' after '/'
	while(true) {
				*first_char = fgetc(source_file);
				if(lookAhead(source_file) == EOF){
					return false;
				}
				if( *first_char == '*' && lookAhead(source_file) == '/') {
					*first_char = fgetc(source_file);
					*first_char = fgetc(source_file);
					break;
				}
			}
	return true;
}

static bool is_func(){
	char c = fgetc(source_file);
	if(c == '(' || (c == ' ' && (lookAhead(source_file) == '('))){
        fseek(source_file,-1,SEEK_CUR);
		return true;
	}
	if(c == ' '){
		while(c == ' ')
			c = fgetc(source_file);

		if(c == '('){
            fseek(source_file,-1,SEEK_CUR);
			return true;
        }
	}
	fseek(source_file,-1,SEEK_CUR);
	return false;
}


Token getToken () {
	
	char first_char = fgetc(source_file);
	while(first_char != EOF){
		bool dropped_into_one = false;
		/*Skipping spaces*/
        
		if(first_char == ' ') {
			dropped_into_one = true;
			first_char = fgetc(source_file);
		}
		/*Skipping tabs*/
		if(first_char == '\t') {
			dropped_into_one = true;
			first_char = fgetc(source_file);
		}
		/*One-line comments filter*/
		if(first_char == '/' && lookAhead(source_file) == '/') {
			dropped_into_one = true;
			bool err = process_line_comment(&first_char);
			if(!err)
				return ErrorToken;
		}
		/*Multiple line comment fitler*/
		if(first_char == '/' && lookAhead(source_file) == '*') {
			dropped_into_one = true;
			bool err = process_multiple_line_comment(&first_char);
			if(!err)
				return ErrorToken;
		}
		if(!dropped_into_one)
			break;

	}



	Mode mode = MODE_UNDEFINED;

	if(first_char >= '0' && first_char <= '9')
		mode = MODE_NUMBER;

	else if (is_letter_or_underscore_or_number(first_char))
		mode = MODE_IDENTIFIER_KEYWORD;

	else {
		switch (first_char) {
			case '\n': mode = MODE_NEW_LINE; break;
			case 13: mode = MODE_CR; break;
			case '+': mode = MODE_PLUS; break;
			case '-': mode = MODE_MINUS; break;
			case '*': mode = MODE_MULTIPLY; break;
			case '/': mode = MODE_DIVIDE; break;
			case ':': mode = MODE_DEFINE; break;
			case '(': mode = MODE_BRACKET; break;
			case ')': mode = MODE_BRACKET; break;
			case '{': mode = MODE_BRACKET; break;
			case '}': mode = MODE_BRACKET; break;
			case '>': mode = MODE_GREATER; break;
			case '<': mode = MODE_LESS; break;
			case '!': mode = MODE_NOT_EQUAL; break;
			case '=': mode = MODE_ASIGN; break;
			case '\"': mode = MODE_STRING; break;
			case ';': mode = MODE_SEMICOLON; break;
			case ',': mode = MODE_COLON; break;
			case EOF: mode = MODE_EOF; break;
			default: mode = MODE_UNDEFINED;
		}
	}
	if (mode == MODE_NUMBER ) {
		Token newToken;
		newToken.type = TOKEN_INT;
		dyn_string_t new_string = str_init();
		
		char curr_char = first_char;
		char next_char = lookAhead(source_file);
		if(curr_char == '0' && (next_char >= '0' && next_char <= '9' ))
			return ErrorToken;
		bool exponential_already = false;
		bool float_already = false;

		while ((next_char >= '0' && next_char <= '9') || next_char == 'e' || next_char == 'E' || next_char == '.') {
			/*error handling*/
			if (exponential_already && next_char == 'e') {
				/*DOUBLE e IN NUMBER LIKE 1.24e20e20*/
				return ErrorToken;
			}
			else if (float_already && next_char == '.') {
				/*DOUBLE . IN NUMBER LIKE 1.24.1 OR 1.24E10.3*/
				return ErrorToken;
			}
			else if(exponential_already && next_char == '.') {
				return ErrorToken;
			}
			
			/*saving curr char to word*/
			
			str_append(&new_string,curr_char);

			if (next_char == '.') {
				newToken.type = TOKEN_FLOAT;
				float_already = true;
				str_append(&new_string,fgetc(source_file));
				next_char = lookAhead(source_file);
				if(next_char == 'e') {
					newToken.type = TOKEN_FLOAT_EXP;
					exponential_already = true;
				}
			}
			else if (newToken.type == TOKEN_FLOAT && (next_char == 'e' || next_char == 'E')){
				newToken.type = TOKEN_FLOAT_EXP;
				exponential_already = true;
				str_append(&new_string,fgetc(source_file));
				next_char = lookAhead(source_file);

				if (next_char == '+') {
					str_append(&new_string,fgetc(source_file));
					next_char = lookAhead(source_file);
				}
				else if (next_char == '-') {
					str_append(&new_string,fgetc(source_file));
					next_char = lookAhead(source_file);
				}
				else 
					str_append(&new_string,'+');
			}
			else if (newToken.type != TOKEN_FLOAT && (next_char == 'e' || next_char == 'E')) {
				newToken.type = TOKEN_INT_EXP;
				exponential_already = true;
				str_append(&new_string,fgetc(source_file));
				next_char = lookAhead(source_file);
				
				if (next_char == '+') {
					str_append(&new_string,fgetc(source_file));
					next_char = lookAhead(source_file);
				}
				else if (next_char == '-') {
					str_append(&new_string,fgetc(source_file));
					next_char = lookAhead(source_file);
				}
				else 
					str_append(&new_string,'+');
			}
			
			curr_char = fgetc(source_file);
			next_char = lookAhead(source_file);
		}
		//number can be immediatly followed only by characters: ' ', <,>,',',=,!,\n
		if(next_char != 13 && next_char != ' ' && next_char != '>' && next_char != '<' &&
		   next_char != '=' && next_char != '!' && next_char != '\n' && next_char != ',' &&
		   next_char != ')' && next_char != '*' && next_char != '/' && next_char != '-' &&
		   next_char != '+' && next_char != ';'&& next_char != '\t'){
			return ErrorToken;
		}
		if(curr_char >= '0' && curr_char <= '9') 
			str_append(&new_string,curr_char);
		
		char* errPtr=NULL;
		if(newToken.type == TOKEN_INT) {
			newToken.data.integer = strtol(new_string.string,&errPtr,10);

		}
		else {
			newToken.data.dbl= strtod(new_string.string,&errPtr);
		}
		str_free(&new_string);
		
		newToken.pointer = NULL;
		return newToken;
	}
	else if (mode == MODE_IDENTIFIER_KEYWORD) { 
		dyn_string_t new_string = str_init();
		str_append(&new_string,first_char);
		char next_char = lookAhead(source_file);
		char curr_char;
		for(int i = 1; is_letter_or_underscore_or_number(next_char); i++) {
			curr_char = fgetc(source_file);
			next_char = lookAhead(source_file);
			str_append(&new_string,curr_char);
		}
		Token *myToken = malloc(sizeof(Token));
		if(isKeyword(new_string.string)) {
			if (myStringCMP(new_string.string,"if")) myToken->type = TOKEN_IF;
			else if (myStringCMP(new_string.string,"else")) myToken->type = TOKEN_ELSE;
			else if (myStringCMP(new_string.string,"package")) myToken->type = TOKEN_PACKAGE;
			else if (myStringCMP(new_string.string,"func")) myToken->type = TOKEN_FUNC;
			else if (myStringCMP(new_string.string,"return")) myToken->type = TOKEN_RETURN;
			else if (myStringCMP(new_string.string,"print")) myToken->type = TOKEN_PRINT;
			else if (myStringCMP(new_string.string,"for")) myToken->type = TOKEN_FOR;
			else if (myStringCMP(new_string.string,"inputi")) myToken->type = TOKEN_INPUT_INT;
			else if (myStringCMP(new_string.string,"inputf")) myToken->type = TOKEN_INPUT_FLOAT;
			else if (myStringCMP(new_string.string,"inputs")) myToken->type = TOKEN_INPUT_STRING;
			else if (myStringCMP(new_string.string,"int")) myToken->type = TOKEN_KEYWORD_INT;
			else if (myStringCMP(new_string.string,"float64")) myToken->type = TOKEN_KEYWORD_FLOAT;
			else if (myStringCMP(new_string.string,"string")) myToken->type = TOKEN_KEYWORD_STRING;
			else if (myStringCMP(new_string.string,"main")) myToken->type = TOKEN_MAIN_FUNC;
			else if (myStringCMP(new_string.string,"int2float")) myToken->type = TOKEN_INT2FLOAT;
			else if (myStringCMP(new_string.string,"float2int")) myToken->type = TOKEN_FLOAT2INT;
			else if (myStringCMP(new_string.string,"len")) myToken->type = TOKEN_LEN;
			else if (myStringCMP(new_string.string,"substr")) myToken->type = TOKEN_SUBSTR;
			else if (myStringCMP(new_string.string,"ord")) myToken->type = TOKEN_ORD;
			else if (myStringCMP(new_string.string,"chr")) myToken->type = TOKEN_CHR;
			dyn_str_cpy(&(myToken->data.string),&new_string);
			str_free(&new_string);
			insert_to_dyn_list(list_of_dyn_strings,myToken);
			return *myToken;
		}
		if(is_func()) { 
			myToken->type = TOKEN_VAR_FUNC;
			dyn_str_cpy(&(myToken->data.string),&new_string);
			insert_to_dyn_list(list_of_dyn_strings,myToken);
		} else {
			if(new_string.string[0] == '_' && new_string.string[1] == '\0') {
				myToken->type = TOKEN_VAR_UNDERSCORE;
			}
			else {
				dyn_str_cpy(&(myToken->data.string),&new_string);
				myToken->type = TOKEN_VAR;
				insert_to_dyn_list(list_of_dyn_strings,myToken);
			}
		}
		
		str_free(&new_string);
		return *myToken;
	}
	else if (mode == MODE_NEW_LINE || mode == MODE_CR) {
		Token myToken = {.type = TOKEN_NEW_LINE};
		if (mode == MODE_CR) {
			first_char = fgetc(source_file); //getting rid off \n and CR
		}
		line_counter++;
		return myToken;
	}
	else if (mode ==  MODE_PLUS) {
		Token myToken = {.type = TOKEN_PLUS};
		return myToken;
	}
	else if (mode == MODE_MULTIPLY) {
		Token myToken = {.type = TOKEN_MULTIPLY};
		return myToken;
	}
	else if (mode == MODE_MINUS) {
		Token myToken = {.type = TOKEN_MINUS};
		return myToken;
	}
	else if (mode == MODE_DIVIDE) {
		Token myToken = {.type = TOKEN_DIVISION};
		return myToken;
	}
	else if (mode == MODE_GREATER) { 
		Token myToken;
		if(lookAhead(source_file) == '=') {
			myToken.type = TOKEN_GREATER_EQUAL;
			fseek(source_file,1,SEEK_CUR);
		}
		else
			myToken.type = TOKEN_GREATER;
		return myToken;
	}
	else if (mode == MODE_LESS) {
		Token myToken;
		if(lookAhead(source_file) == '=') {
			myToken.type = TOKEN_LESS_EQUAL;
			fseek(source_file,1,SEEK_CUR);
		}
		else
			myToken.type = TOKEN_LESS;
		return myToken;
	}
	else if (mode == MODE_NOT_EQUAL) {
		if(lookAhead(source_file) == '=') {
			fseek(source_file,1,SEEK_CUR);
			Token myToken = {.type = TOKEN_NOT_EQUAL};
			return myToken;
		}
		else {
		
			return ErrorToken;
		}
	}
	else if (mode == MODE_DEFINE) { 
		if(lookAhead(source_file) == '=') {
			fseek(source_file,1,SEEK_CUR);
			Token myToken = {.type = TOKEN_DEFINE};
			return myToken;
		}
		else {
			return ErrorToken;
		}
	}
	else if (mode == MODE_BRACKET) {
		Token myToken;
		if(first_char == '{') myToken.type = TOKEN_LEFT_CURLY;
		else if (first_char == '}') myToken.type = TOKEN_RIGHT_CURLY;
		else if (first_char == '(') myToken.type = TOKEN_LEFT_ROUND;
		else if (first_char == ')') myToken.type = TOKEN_RIGHT_ROUND;
		return myToken;
	}
	else if (mode ==  MODE_EOF) {
		Token myToken = {.type = TOKEN_EOF};
		return myToken;
	}
	else if (mode == MODE_UNDEFINED) {
		Token myToken;
		myToken.type = TOKEN_UNDEFINED;
		return myToken;
	}
	else if (mode == MODE_STRING) {
		dyn_string_t new_string = str_init();
		Token *myToken = malloc(sizeof(Token));
		myToken->type = TOKEN_STRING;
		

		if(lookAhead(source_file) == '\"') {
			//Empty string
			dyn_str_cpy(&(myToken->data.string),&new_string);
			str_free(&new_string);
			insert_to_dyn_list(list_of_dyn_strings,myToken);
			fseek(source_file,SEEK_CUR,1);
			return *myToken;
		}

		char curr_char;
        char next_char = lookAhead(source_file);
		int i;
		for (i = 1; next_char != '\"'; i++) {
			curr_char = fgetc(source_file);
            next_char = lookAhead(source_file);
            if(curr_char == '\\') { //Handling the escape expression (\) expressions like (\n,\t,\")
                str_append(&new_string,curr_char); 
                bool error = process_backslash(&new_string);
				if(error){
					str_free(&new_string);
					return ErrorToken;
				}
				next_char = lookAhead(source_file);
            } else {
                str_append(&new_string,curr_char); 
            }
			if (next_char == '\n' || next_char == EOF) {
				return ErrorToken;
			}
		}

		fseek(source_file,SEEK_CUR,1);
		dyn_str_cpy(&(myToken->data.string),&new_string);
		str_free(&new_string);
		insert_to_dyn_list(list_of_dyn_strings,myToken);
		return *myToken;
	}
	else if (mode == MODE_SEMICOLON) {
		Token myToken = {.type = TOKEN_SEMICOLON};
		return myToken;
	}
	else if (mode == MODE_COLON) {
		Token myToken = {.type = TOKEN_COLON};
		return myToken;
	}
	else if (mode == MODE_ASIGN) {
		if(lookAhead(source_file) == '='){
			fseek(source_file,1,SEEK_CUR);
			Token myToken = {.type = TOKEN_EQUAL};
			return myToken;
		}
		Token myToken = {.type = TOKEN_ASSIGN};
		return myToken;
	}
	
    return ErrorToken;
}

bool is_letter_or_underscore_or_number(char c) { 
	if (c == '_' || (c >= 'A' && c <= 'Z') ||  (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))
		return true;
	return false;
}

bool myStringCMP(char *string1, char *string2) {
    int length = strlen(string1);
	int length2 = strlen(string2);
	if(length2 != length)
		return false;
    for (int i = 0; i < length; i++) {
        if(string1[i] != string2[i]) {
            return false;
        }
    }
	return true;
}

bool isKeyword (char* word){

	if( myStringCMP(word,"package") || myStringCMP(word, "func") || myStringCMP(word,"return") ||
        myStringCMP(word,"if") || myStringCMP(word,"print") || myStringCMP(word,"else") ||
        myStringCMP(word,"inputi") || myStringCMP(word,"inputs") ||myStringCMP(word,"inputf") ||
		myStringCMP(word,"else") || myStringCMP(word,"for") || myStringCMP(word,"int") || 
		myStringCMP(word,"float64") || myStringCMP(word,"string") || myStringCMP(word,"main")|| /*from here*/
        myStringCMP(word,"int2float") || myStringCMP(word,"float2int") || myStringCMP(word,"len") ||
        myStringCMP(word,"substr") || myStringCMP(word,"ord") || myStringCMP(word,"chr") )
        return true;
    return false;
}

void clearList(char* list, int length) {
	for(int i = 0; i < length; i++)
		list[i] = '\0';
}

bool process_backslash(dyn_string_t *word){
	
	char next_char = fgetc(source_file);
	if(!(next_char == 'a' || next_char == 'b' || next_char == 'f' || next_char == 'n' || next_char == 'r' || next_char == 't' || next_char == 'v' ||
		 next_char == '\\' || next_char == '\'' || next_char == '"' || next_char == '?' || next_char == '0' || next_char == 'x' || next_char == '0' )) {
			 return true;
		 }

	str_append(word,next_char);
	if(next_char == '0'){
		next_char = fgetc(source_file);
		if(next_char >= '0' && next_char <= '7'){
			str_append(word,next_char);
		}
		else {
			return true;
		}
		next_char = fgetc(source_file);
		if(next_char >= '0' && next_char <= '7'){
			str_append(word,next_char);
		}
		else {
			return true;
		}
	} else if (next_char == 'x') {
		
		next_char = fgetc(source_file);
		if((next_char >= '0' && next_char <= '9') || (next_char >= 'a' && next_char <= 'f')|| (next_char >= 'A' && next_char <= 'F')){
			str_append(word,next_char);
		} else {
			return true;
		}
		next_char = fgetc(source_file);
		if((next_char >= '0' && next_char <= '9') || (next_char >= 'a' && next_char <= 'f')|| (next_char >= 'A' && next_char <= 'F')){
			str_append(word,next_char);
		} else {
			return true;
		}
	}

	return false;
}


void print_token(Token newToken,FILE* print_file) {

    if (newToken.type == TOKEN_INT) {
        fprintf(print_file,"TOKEN_INT--------");
        fprintf(print_file,"%" PRId64 "\n", newToken.data.integer);
    }
    else if (newToken.type == TOKEN_VAR_FUNC) {
        fprintf(print_file,"TOKEN_VAR_FUNC, name of function - %s\n",newToken.data.string.string);
    }
    else if (newToken.type == TOKEN_INT_EXP) {
        fprintf(print_file,"TOKEN_INT_EXP-----%lf\n",newToken.data.dbl);
    }
    else if (newToken.type == TOKEN_FLOAT) {
        fprintf(print_file,"TOKEN_FLOAT-----%lf\n",newToken.data.dbl);
    }
    else if (newToken.type == TOKEN_FLOAT_EXP) {
        fprintf(print_file,"TOKEN_FLOAT_EXP-----%lf\n",newToken.data.dbl);
    }
    else if (newToken.type == TOKEN_STRING) {
        fprintf(print_file,"TOKEN_STRING----%s \n",newToken.data.string.string);
    }
    else if (newToken.type == TOKEN_VAR) {
        fprintf(print_file,"TOKEN_VAR, name of variable = %s\n",newToken.data.string.string);
    }
    else if (newToken.type == TOKEN_VAR_UNDERSCORE) {
        fprintf(print_file,"TOKEN_VAR_UNDERSCORE\n");
    }
    else if (newToken.type == TOKEN_FUNC) {
        fprintf(print_file,"TOKEN_FUNC\n");
    }
    else if (newToken.type == TOKEN_PACKAGE) {
        fprintf(print_file,"TOKEN_PACKAGE\n");
    }
    else if (newToken.type == TOKEN_RETURN) {
        fprintf(print_file,"TOKEN_RETURN\n");
    }
    else if (newToken.type == TOKEN_PRINT) {
        fprintf(print_file,"TOKEN_PRINT\n");
    }
    else if (newToken.type == TOKEN_INPUT_INT) {
        fprintf(print_file,"TOKEN_INPUT_INT\n");
    }
    else if (newToken.type == TOKEN_INPUT_FLOAT) {
        fprintf(print_file,"TOKEN_INPUT_FLOAT\n");
    }
    else if (newToken.type == TOKEN_INPUT_STRING) {
        fprintf(print_file,"TOKEN_INPUT_STRING\n");
    }
    else if (newToken.type == TOKEN_FOR) {
        fprintf(print_file,"TOKEN_FOR\n");
    }
    else if (newToken.type == TOKEN_IF) {
        fprintf(print_file,"TOKEN_IF\n");
    }
    else if (newToken.type == TOKEN_WHILE) {
        fprintf(print_file,"TOKEN_WHILE\n");
    }
    else if (newToken.type == TOKEN_ELSE) {
        fprintf(print_file,"TOKEN_ELSE\n");
    }
    else if (newToken.type == TOKEN_IF_ELSE) {
        fprintf(print_file,"TOKEN_IF_ELSE\n");
    }
    else if (newToken.type == TOKEN_NEW_LINE) {
        fprintf(print_file,"TOKEN_NEW_LINE\n");
    }
    else if (newToken.type == TOKEN_SEMICOLON) {
        fprintf(print_file,"TOKEN_SEMICOLON\n");
    }
    else if (newToken.type == TOKEN_COLON) {
        fprintf(print_file,"TOKEN_COLON\n");
    }
    else if (newToken.type == TOKEN_PLUS) {
        fprintf(print_file,"TOKEN_PLUS\n");
    }
    else if (newToken.type == TOKEN_MULTIPLY) {
        fprintf(print_file,"TOKEN_MULTIPLY\n");
    }
    else if (newToken.type == TOKEN_MINUS) {
        fprintf(print_file,"TOKEN_MINUS\n");
    }
    else if (newToken.type == TOKEN_DIVISION) {
        fprintf(print_file,"TOKEN_DIVISION\n");
    }
    else if (newToken.type == TOKEN_ASSIGN) {
        fprintf(print_file,"TOKEN_ASSIGN\n");
    }
    else if (newToken.type == TOKEN_DEFINE) {
        fprintf(print_file,"TOKEN_DEFINE\n");
    }
    else if (newToken.type == TOKEN_EQUAL) {
        fprintf(print_file,"TOKEN_EQUAL\n");
    }
    else if (newToken.type == TOKEN_NOT_EQUAL) {
        fprintf(print_file,"TOKEN_NOT_EQUAL\n");
    }
    else if (newToken.type == TOKEN_LESS) {
        fprintf(print_file,"TOKEN_LESS\n");
    }
    else if (newToken.type == TOKEN_LESS_EQUAL) {
        fprintf(print_file,"TOKEN_LESS_EQUAL\n");
    }
    else if (newToken.type == TOKEN_GREATER) {
        fprintf(print_file,"TOKEN_GREATER\n");
    }
    else if (newToken.type == TOKEN_GREATER_EQUAL) {
        fprintf(print_file,"TOKEN_GREATER_EQUAL\n");
    }
    else if (newToken.type == TOKEN_LEFT_ROUND) {
        fprintf(print_file,"TOKEN_LEFT_ROUND\n");
    }
    else if (newToken.type == TOKEN_RIGHT_ROUND) {
        fprintf(print_file,"TOKEN_RIGHT_ROUND\n");
    }
    else if (newToken.type == TOKEN_LEFT_CURLY) {
        fprintf(print_file,"TOKEN_LEFT_CURLY\n");
    }
    else if (newToken.type == TOKEN_RIGHT_CURLY) {
        fprintf(print_file,"TOKEN_RIGHT_CURLY\n");
    }
    else if (newToken.type == TOKEN_EOF) {
        fprintf(print_file,"TOKEN_EOF\n");
    }
    else if (newToken.type == TOKEN_UNDEFINED) {
        fprintf(print_file,"TOKEN_UNDEFINED \n");
    }
	else if (newToken.type == TOKEN_KEYWORD_INT) {
        fprintf(print_file,"TOKEN_KEYWORD_INT\n");
    }
	else if (newToken.type == TOKEN_KEYWORD_FLOAT) {
        fprintf(print_file,"TOKEN_KEYWORD_FLOAT\n");
    }
	else if (newToken.type == TOKEN_KEYWORD_STRING) {
        fprintf(print_file,"TOKEN_KEYWORD_STRING\n");
    }
	else if (newToken.type == TOKEN_MAIN_FUNC) {
        fprintf(print_file,"TOKEN_MAIN_FUNC\n");
    }  else if (newToken.type == TOKEN_DOLLAR) {
        fprintf(print_file,"TOKEN_DOLLAR\n");
    } else if (newToken.type == TOKEN_NON_TERM) {
        fprintf(print_file,"TOKEN_NON_TERM\n");
    } else if (newToken.type == TOKEN_SHIFT) {
        fprintf(print_file,"TOKEN_SHIFT\n");
    } else if (newToken.type == TOKEN_INT2FLOAT) {
        fprintf(print_file,"TOKEN_INT2FLOAT\n");
    } else if (newToken.type == TOKEN_FLOAT2INT) {
        fprintf(print_file,"TOKEN_FLOAT2INT\n");
    } else if (newToken.type == TOKEN_LEN) {
        fprintf(print_file,"TOKEN_LEN\n");
    } else if (newToken.type == TOKEN_SUBSTR) {
        fprintf(print_file,"TOKEN_SUBSTR\n");
    } else if (newToken.type == TOKEN_ORD) {
        fprintf(print_file,"TOKEN_ORD\n");
    } else if (newToken.type == TOKEN_CHR) {
        fprintf(print_file,"TOKEN_CHR\n");
    }
	

}
