/**
 * parser.c - Syntactical analysis of code
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

#include "parser.h"

Token actual_token;
char key[256];

char return_typ[256];
int index_return;
FILE *my_print_file;
node_t actual_error_node;
node_t *error_node; 

// premenna ak zozralo token eps pravidlo
bool eaten_token = false;
int counter_type = 0;
bool in_define = false;
bool in_assign = false;
bool for_init_empty = false;
bool is_underscore = false;
bool is_another_underscore = false;
bool use_main = false;
bool in_main = false;
bool multiple = false;


bool in_param = false;
bool in_return = false;

//index_return = 0;
int index_param = 0;



Token tmp_token;
Token token_for_type;

// Function skip all TOKEN_NEW_LINE in row and return new token                    
Token get_new_token (bool skip_new_line)
{
   // ak zozralo token eps pravidlo actual token zostava nezmeneny
   if (eaten_token)
   {
       eaten_token = false;
   }
   else
   {
        actual_token = getToken();
        
        if (skip_new_line == true)
        {
            while (actual_token.type == TOKEN_NEW_LINE)
            {
                actual_token = getToken();
            }
        }
    }
    /* print_token(actual_token, stdout);  // DEBUG */
    return actual_token;
}



/* ****************
 * <prolog>
 * ***************/
int prolog(stab_dict_t dict)
{
    get_new_token(true);
    if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;

    // <prolog> -> package main eol <prog>
    if (actual_token.type != TOKEN_PACKAGE)                                                         // -> package 
    {
        return SYNTAX_ERROR;
    }

    get_new_token(false);
    if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;

    if (actual_token.type != TOKEN_MAIN_FUNC)                                                       // -> main    
    {
        return SYNTAX_ERROR;
    } 
    get_new_token(false);
    if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;
    
    if (actual_token.type != TOKEN_NEW_LINE)                                                        // -> eol
    {
        return SYNTAX_ERROR; 
    }
    return prog(dict);                                                                              // -> <prog>

}




/* ****************
 * <prog>
 * ***************/
int prog(stab_dict_t dict)
{
    int result;

    get_new_token(true);
    if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;   

    switch(actual_token.type)
    {
        //  <prog> -> func <id_func> <scope> <prog>   
        case TOKEN_FUNC :                                                                           // -> func
            ;
            root_t scope_func = stab_root_init(dict);  //! vytvorim tabulku
            result = id_func(dict, scope_func);                                                     // -> <id_func>
            if (result != SYNTAX_OK) return result;

            codegen_params(NULL, true);
            result = scope(dict, scope_func);                                                       // -> <scope>
            if (in_main) {
               codegen_end_func();
               printf("JUMP END\n");
               in_main = false;
            } else
               codegen_ret_func();
            
            stab_dict_pop(dict);
            stab_destroy(scope_func);                 //! vymazem tabulku func
            
            if (result != SYNTAX_OK) return result;
    
            return prog(dict);                                                                      // -> <prog>
        //    
 
        //  <prog> -> eof
        case TOKEN_EOF :                                                                            // -> eof
            if (use_main)
                return SYNTAX_OK;   // ak bol eof uspeny koniec
            else
                return DEFINITION_ERROR;
                 
         default:
            break;
    }
      
    return SYNTAX_ERROR;    // ak ani jeden z switch
    
}



/* ****************
 * <id_func>
 * ***************/
int id_func(stab_dict_t dict, root_t scope_table)
{   
    int result;

    get_new_token(false);
    if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;
    

    switch(actual_token.type)
    {
        //  <id_func> -> id_func ( <param> ) <return_type>
        case TOKEN_VAR_FUNC :                                                                       // -> id_func
            strcpy(key, actual_token.data.string.string);//!
            
            // kontrola ci tam uz nie je funkcia 
            if(stab_lookup(dict->global->root,key))
            {
                node_t* tmp1 = stab_get(dict->global->root,key);
                // kontrola ci bola aj deklarovana || volana este pred deklaraciu
                if(tmp1->declared)
                    return DEFINITION_ERROR;          
            } 
            else // ak nebola vlozime
            {
                stab_insert(dict->global->root,key);
                node_t* tmp_param;
                tmp_param = stab_get(dict->global->root,key);
                tmp_param->declared = true;
            }
            //

            codegen_new_func(key, 0);
            get_new_token(false);
            if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;
     
            if (actual_token.type != TOKEN_LEFT_ROUND)                                              // -> (
            {
                return SYNTAX_ERROR;
            }
            in_param = true;
            result = param(dict, scope_table);                                                      // -> <param> 
            if (result != SYNTAX_OK) return result;

            // token nacitany v <param> -> eps
            if (actual_token.type != TOKEN_RIGHT_ROUND)                                             // -> )
            {
                return SYNTAX_ERROR;
            }
            in_param = false;

            in_return = true;
            result = return_type(dict, scope_table);                                                // -> <return_type>
            if (result != SYNTAX_OK) return result;
            in_return = false;

            node_t* tmp2 = stab_get(dict->global->root,key);
           
            if (!tmp2->declared)
            {
                if (tmp2->return_count != index_return) return RETURN_ERROR;

                if (tmp2->param_count != index_param) return PARAM_CNT_ERROR;
                
                
                tmp2->declared = true; //! asi 
                
            }
            return SYNTAX_OK;       // DEBUG iba teraz 
        //


        // <id_func> -> main ( )
        case TOKEN_MAIN_FUNC :                                                                      // -> main
            // kontrola redefinicie mainu
            if (use_main == false)
                use_main = true;
            else return DEFINITION_ERROR;
            
            in_main = true;
            codegen_new_func("main", 0);
            get_new_token(false);
            if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;

            if (actual_token.type != TOKEN_LEFT_ROUND)                                              // -> (
            {
                return  SYNTAX_ERROR;
            }

            get_new_token(false);
            if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;
        
            if (actual_token.type != TOKEN_RIGHT_ROUND)                                             // -> )
            {
                return SYNTAX_ERROR;
            }

            return SYNTAX_OK;
        //    
         default:
            break;
    }
    return SYNTAX_ERROR;    // token != id_func || main
}



/* ****************
 * <param>
 * ***************/
int param(stab_dict_t dict, root_t scope_table)
{
    int result;
    
    index_param = 0;

    get_new_token(true);
    if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;
    

    switch(actual_token.type)
    {
        //  <param> -> id <type_without_eps> <param_n>
        case TOKEN_VAR :                                                                            // -> id

            // kontrola ci sa nejdena o redifiniciu
            if (stab_lookup(scope_table, actual_token.data.string.string)) return DEFINITION_ERROR;
        
            stab_insert(scope_table, actual_token.data.string.string); // vlozim to table variable
            codegen_params(stab_get(scope_table, actual_token.data.string.string), false);
            // ulozim si ID variable do pomocnej
            tmp_token = actual_token;

            // volanie tokenu pre typ_without_eps
            get_new_token(false);    // nemoze tu byt eol 
            if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;
           
            result = type_without_eps(dict,scope_table);                                            // -> <type_without_eps>
            if (result != SYNTAX_OK) return result;     

            result = param_n(dict, scope_table);                                                    // -> <param_n>
            if (result != SYNTAX_OK) return result;     

            return SYNTAX_OK;
         default:
            break;
    }

    //  <param> -> eps
    return SYNTAX_OK;  
}




/* ****************
 * <param_n>
 * ***************/
int param_n(stab_dict_t dict, root_t scope_table)
{
    int result;

    get_new_token(false);
    if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;

    switch(actual_token.type)
    {
        //  <param_n> -> , id <type_without_eps> <param_n>
        case TOKEN_COLON :                                                                          // -> ,
            get_new_token(true); // moze byt EOL za ciarkov
            if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;
         
            if (actual_token.type != TOKEN_VAR)                                                     // -> id
            {
                return SYNTAX_ERROR;
            }
            // kontrola ci sa nejdena o redifiniciu
            if (stab_lookup(scope_table, actual_token.data.string.string)) return DEFINITION_ERROR;

            stab_insert(scope_table, actual_token.data.string.string); // vlozim to table variable
            codegen_params(stab_get(scope_table, actual_token.data.string.string), false);
           
            tmp_token = actual_token; // ulozim si ID variable do pomocnej
           
            // volanie tokenu pre typ_without_eps
            get_new_token(false);    // nemoze tu byt eol 
            if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;
              
            result = type_without_eps(dict,scope_table);                                            // -> <type_without_eps>
            if (result != SYNTAX_OK) return result;     
            

            result = param_n(dict, scope_table);                                                    // -> <param_n>
            if (result != SYNTAX_OK) return result;     

            return SYNTAX_OK;
        //    
         default:
            break;
    }

    //  <param_n> -> eps
    return SYNTAX_OK;
}



/* ****************
 * <return_type>
 * ***************/
int return_type(stab_dict_t dict, root_t scope_table)
{
    int result;
    index_return = 0;

    get_new_token(false);
    if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;

    switch(actual_token.type)
    {
        //  <return_type> -> ( <type> )
        case TOKEN_LEFT_ROUND :                                                                     // -> (
            result = type(dict, scope_table);                                                       // -> <type> 
            if (result != SYNTAX_OK) return result;
           
            // token nacitalo v <type> -> eps
            if (actual_token.type != TOKEN_RIGHT_ROUND)                                             // -> )
            {
                return SYNTAX_ERROR;
            }
            return SYNTAX_OK;   
        //         
         default:
            break;
    }

    //  <return_type> -> eps
    eaten_token = true;
    return SYNTAX_OK;
        
}




/* ****************
 * <scope>
 * ***************/
int scope(stab_dict_t dict, root_t scope_table)
{
    int result;

    get_new_token(false);   // nemoze tu byt odriadkovanie preto false
    if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;

    //  <scope> -> { eol <statement> }
    if (actual_token.type != TOKEN_LEFT_CURLY)                                                      // -> {
    {
        return SYNTAX_ERROR;
    }
    get_new_token(false);
    if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;
   
    if (actual_token.type != TOKEN_NEW_LINE)                                                        // -> eol
    {
        return SYNTAX_ERROR;
    }
    result = statement(dict, scope_table);                                                          // -> <statement>
    if (result != SYNTAX_OK) return result;

    get_new_token(true);       // mozu tu byt new_line
    if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;
    
    if (actual_token.type != TOKEN_RIGHT_CURLY)                                                     // -> }
    {
        return SYNTAX_ERROR;
    }

    return SYNTAX_OK;   
}



/* ****************
 * <type>
 * ***************/
int type(stab_dict_t dict, root_t scope_table)
{
    int result;

    get_new_token(false);
    if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;

    switch(actual_token.type)
    {   
        //  <type> -> integer <type_n>
        case TOKEN_KEYWORD_INT :                                                                    // -> integer
            if (in_return)
            {
                node_t* tmp_return;
                tmp_return = stab_get(dict->global->root,key);

                if (tmp_return->declared) // pokial definova pred mainom
                {
                    tmp_return->return_types[index_return] = 'i';
                    index_return ++;
                    tmp_return->return_count = index_return;
                }
                else // definovana za mainom 
                {
                    if (!(tmp_return->return_types[index_return] == 'i'))
                    {
                        if (tmp_return->return_types[index_return] != '_')
                            return RETURN_ERROR;
                    }
                    index_return ++;
                }
            }
            result = type_n(dict, scope_table);                                                     // -> <type_n>
            if (result != SYNTAX_OK) return result;
            return SYNTAX_OK;


        //  <type> -> float <type_n>
        case TOKEN_KEYWORD_FLOAT :                                                                  // -> float
            if (in_return)
            {
                node_t* tmp_return;
                tmp_return = stab_get(dict->global->root,key);
                //printf("KLUC: %s",key);
                if (tmp_return->declared) // pokial definova pred mainom
                {
                    tmp_return->return_types[index_return] = 'f';
                    index_return ++;
                    tmp_return->return_count = index_return;
                }
                else // definovana za mainom 
                {
                    if (!(tmp_return->return_types[index_return] == 'f'))
                    {
                        if (tmp_return->return_types[index_return] != '_')
                            return RETURN_ERROR;
                    } 
                    index_return ++;
                }
            }
            result = type_n(dict, scope_table);                                                     // -> <type_n>
            if (result != SYNTAX_OK) return result;
            return SYNTAX_OK;

        //  <type> -> string <type_n>
        case TOKEN_KEYWORD_STRING :                                                                 // -> string
            if (in_return)
            {
                node_t* tmp_return;
                tmp_return = stab_get(dict->global->root,key);
                //printf("KLUC: %s",key);
                if (tmp_return->declared) // pokial definova pred mainom
                {
                    tmp_return->return_types[index_return] = 's';
                    index_return ++;
                    tmp_return->return_count = index_return;
                }
                else // definovana za mainom 
                {
                    if (!(tmp_return->return_types[index_return] == 's'))
                    {
                        if (tmp_return->return_types[index_return] != '_')
                            return RETURN_ERROR;
                    } 
                    index_return ++;
                }
            }
            result = type_n(dict, scope_table);                                                     // -> <type_n>
            if (result != SYNTAX_OK) return result;
            return SYNTAX_OK;
         default:
            break;
    }

    //  <type> -> eps
    return SYNTAX_OK;   // DEBUG teraz
}



/* ****************
 * <type_without_eps>
 * ***************/
int type_without_eps(stab_dict_t dict, root_t scope_table)
{
    switch(actual_token.type)
    {   
        //  <type_without_eps> -> integer
        case TOKEN_KEYWORD_INT :                                                                    // -> integer
            
            if (in_param)
            {
                token_for_type.type = TOKEN_INT;
                stab_set(scope_table,tmp_token.data.string.string,token_for_type.type);
                node_t* tmp_param;
                tmp_param = stab_get(dict->global->root,key);
                
                if (tmp_param->declared)
                {
                    tmp_param->params[index_param] = 'i';
                    index_param ++;
                    tmp_param->param_count = index_param;
                }
                else
                {
                    if (tmp_param->params[index_param] != 'i') return PARAM_CNT_ERROR;

                    index_param ++;
                }
            }
            if (in_return)
            {
                node_t* tmp_return;
           
                tmp_return = stab_get(dict->global->root,key);
                
                if (tmp_return->declared) // pokial definova pred mainom
                {
                    tmp_return->return_types[index_return] = 'i';
                    index_return ++;
                    tmp_return->return_count = index_return;
                }
                else // definovana za mainom 
                {
                    if (!(tmp_return->return_types[index_return] == 'i'))
                    {
                        if (tmp_return->return_types[index_return] != '_')
                            return RETURN_ERROR;
                    } 
                    index_return ++;
                }
            }
            return SYNTAX_OK;
        //    


        //  <type_without_eps> -> float64
        case TOKEN_KEYWORD_FLOAT :                                                                  // -> float64
            
            if (in_param)
            {   
                token_for_type.type = TOKEN_FLOAT;
                stab_set(scope_table,tmp_token.data.string.string,token_for_type.type);
                node_t* tmp_param;
                tmp_param = stab_get(dict->global->root,key);
              
                if (tmp_param->declared)
                {
                    tmp_param->params[index_param] = 'f';
                    index_param ++;
                    tmp_param->param_count = index_param;
                }
                else
                {   
                    if (tmp_param->params[index_param] != 'f') return PARAM_CNT_ERROR;
                    
                    index_param ++;
                }
            }
            if (in_return)
            {
                node_t* tmp_return;
                tmp_return = stab_get(dict->global->root,key);

                if (tmp_return->declared) // pokial definova pred mainom
                {
                    tmp_return->return_types[index_return] = 'f';
                    index_return ++;
                    tmp_return->return_count = index_return;
                }
                else // definovana za mainom 
                {
                    if (!(tmp_return->return_types[index_return] == 'f'))
                    {
                        if (tmp_return->return_types[index_return] != '_')
                            return RETURN_ERROR;
                    }

                    index_return ++;
                }
                
            }
            return SYNTAX_OK;
        //


        //  <type_without_eps> -> string
        case TOKEN_KEYWORD_STRING :                                                                 // -> string
            
            if (in_param)
            {   token_for_type.type = TOKEN_STRING;
                stab_set(scope_table,tmp_token.data.string.string,token_for_type.type);
                node_t* tmp_param;
                tmp_param = stab_get(dict->global->root,key);
              
                if (tmp_param->declared)
                {
                    tmp_param->params[index_param] = 's';
                    index_param ++;
                    tmp_param->param_count = index_param;
                }
                else
                {
                    if (tmp_param->params[index_param] != 's') return PARAM_CNT_ERROR;

                    index_param ++;
                }
            }
            if (in_return)
            {
                node_t* tmp_return;
                tmp_return = stab_get(dict->global->root,key);
              
                if (tmp_return->declared) // pokial definova pred mainom
                {
                    tmp_return->return_types[index_return] = 's';
                    index_return ++;
                    tmp_return->return_count = index_return;
                }
                else // definovana za mainom 
                {
                    if (!(tmp_return->return_types[index_return] == 's'))
                    {
                        if (tmp_return->return_types[index_return] != '_')
                            return RETURN_ERROR;
                    }
                    index_return ++;
                }
            }
            return SYNTAX_OK;
        //    
         default:
            break;
    }
    return SYNTAX_ERROR;
}




/* ****************
 * <type_n>
 * ***************/
int type_n(stab_dict_t dict, root_t scope_table)
{
    int result;

    get_new_token(false);
    if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;

    switch(actual_token.type)
    {   
        //  <type_n> -> , <type_without_eps>
        case TOKEN_COLON :                                                                          // -> ,

            // volanie tokenu pre typ_without_eps
            get_new_token(true);    // moze tu byt eol 
            if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;
           
            result = type_without_eps(dict,scope_table);                                            // -> <type_without_eps>
            if (result != SYNTAX_OK) return result;
           
            // pridane 
            result = type_n(dict,scope_table);
            if (result != SYNTAX_OK) return result;
            return SYNTAX_OK;
        //    
         default:
            break;
    }

    // <type_n> -> eps
    return SYNTAX_OK;
}




/* ****************
 * <statement>
 * ***************/
int statement(stab_dict_t dict, root_t scope_table)
{
    int result;

    get_new_token(true);    // moze byt eol medzi <statement>
    if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;

    switch(actual_token.type)
    {
        //  <statement> -> <function> eol <statement>
        /* case TOKEN_PRINT ... TOKEN_CHR : */
        case TOKEN_PRINT:
        case TOKEN_VAR_FUNC:
        case TOKEN_INPUT_INT:
        case TOKEN_INPUT_FLOAT:
        case TOKEN_INPUT_STRING:
        case TOKEN_INT2FLOAT:
        case TOKEN_FLOAT2INT:
        case TOKEN_LEN:
        case TOKEN_SUBSTR:
        case TOKEN_ORD:
        case TOKEN_CHR:
            in_assign = false;
            result = function(dict, scope_table);                                                   // -> <function>
            if (result != SYNTAX_OK) return result;

            get_new_token(false);
            if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;
   
            if (actual_token.type != TOKEN_NEW_LINE)                                                // -> eol
            {
                return SYNTAX_ERROR;
            }
            return statement(dict, scope_table);                                                    // -> <statement>
        //


        //  <statement> -> return <expr> eol <statement>
        case TOKEN_RETURN :                                                                         // -> return
            get_new_token(false);
            if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;
            
            // osetrenie ci nie je return prazdny
            if (actual_token.type != TOKEN_NEW_LINE)
            {
                eaten_token = true;
                result = expr(dict, scope_table);                                                   // -> <expr>
                if (result != SYNTAX_OK) return result;
            }

            if (actual_token.type != TOKEN_NEW_LINE)                                                // -> eol
            {
                return SYNTAX_ERROR;
            }
            return statement(dict, scope_table);                                                    // -> <statement>
        // 


        //  <statement> -> for <for_init> ; <expresssion> ; <for_init> <scope> eol <statement>
        case TOKEN_FOR :                                                                            // -> for
            ; //! aby islo 
            root_t scope_for_def = stab_root_init(dict); //! vytvorim tabulku for_def

			printf("CREATEFRAME\nPUSHFRAME\n");
            result = for_init(dict, scope_for_def);                                                 // -> <for_init>
            codegen_new_loop();
            
            // kontrola ci definicia vo fore nie je eps
            if (for_init_empty != true)
            {  
                if (in_define != true) return SYNTAX_ERROR;
            } 
            
            if (result != SYNTAX_OK) return result;
           
            // token mi nacitalo <expression> || <for_init> -> eps
            if (actual_token.type != TOKEN_SEMICOLON)                                               // -> ;
            {
                return SYNTAX_ERROR;
            }
            // musim dat novy token <expression> 
            eaten_token = false; // musim dat false pretoze <assign> vo <for_init> dalo true
            get_new_token(false);
            if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;
           
            where = BOOL_TRUE;
            result = expression(dict,scope_for_def,tmp_token);                                      // -> <expression>
            where = BOOL_FALSE;
            codegen_jump_loop();
            if (result != SYNTAX_OK) return result;
          
            // token mi nacitalo <expression>
            if (actual_token.type != TOKEN_SEMICOLON)                                               // -> ;
            {
                return SYNTAX_ERROR;
            }
            result = for_init(dict, scope_for_def);                                                 // -> <for_init>
            
            // kontrola ci definicia vo fore nie je eps
            if (for_init_empty != true)
            {
                if (in_define) return SYNTAX_ERROR;
            }
            
            if (result != SYNTAX_OK) return result;
            
            // token mi nacitalo <expression> || <for_init> -> eps
            eaten_token = true;
            root_t scope_for = stab_root_init(dict);     //! vytvorim tabulku for
            codegen_after_loop(); 
            result = scope(dict, scope_for);                                                        // -> <scope>
            codegen_end_loop();

            stab_dict_pop(dict);
            stab_destroy(scope_for);                    //! vymazem tabulku for
            if (result != SYNTAX_OK) return result;
            
            stab_dict_pop(dict);
            stab_destroy(scope_for_def);                //! vymazem tabulku for_def            

            get_new_token(false);
            if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;
 
            if (actual_token.type != TOKEN_NEW_LINE)                                                // -> eol
            {
                return SYNTAX_ERROR;
            }
            return statement(dict, scope_table);                                                    // -> <statement>
        //



        //  <statement> -> if <expression> <scope> else <scope> eol <statement>
        case TOKEN_IF :                                                                             // -> if
            // gorkymu nacitam token a zavolam expresion()
            codegen_new_if_statement();
            get_new_token(false);
            if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;
           
            where = BOOL_TRUE;
            result = expression(dict,scope_table, tmp_token);                                       // -> <expression>
            where = BOOL_FALSE;
            codegen_if_jump();
            if (result != SYNTAX_OK) return result;

            root_t scope_if = stab_root_init(dict); //! vytvorim tabulku if

            // expression nacitala dalsi token do actual_token
            eaten_token = true; // expression uz nacitala token
            result = scope(dict, scope_if);                                                         // <scope>
            
            stab_dict_pop(dict);
            stab_destroy(scope_if);                 //! vymazem tabulku if

            if (result != SYNTAX_OK) return result;

            get_new_token(false);        
            if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;
  
            if (actual_token.type != TOKEN_ELSE)                                                    // -> else
            {
                return SYNTAX_ERROR;
            }
            codegen_end_if();
            codegen_if();
            root_t scope_else = stab_root_init(dict); //! vytvorim tabulku else

            result = scope(dict, scope_else);                                                       // <scope>
			codegen_end_if_statement();
            stab_dict_pop(dict);
            stab_destroy(scope_else);                 //! vymazem tabulku else
            if (result != SYNTAX_OK) return result;

            get_new_token(false);
            if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;

            if (actual_token.type != TOKEN_NEW_LINE)                                                // -> eol 
            {
                return SYNTAX_ERROR;
            }
            return statement(dict, scope_table);                                                    // -> <statement>
        //


        //  <statement> id <dec_and_def> eol <statement>
        case TOKEN_VAR :                                                                            // -> id 
            is_underscore = false;
            is_another_underscore = false;
            tmp_token = actual_token; //! ulozim si ID
            index_return = 0;

    
            get_new_token(false);
            if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;

            if (actual_token.type == TOKEN_DEFINE)
            {
                // kontrola ci sa nejdena o redifiniciu
                if (stab_lookup(scope_table, tmp_token.data.string.string)) return DEFINITION_ERROR;

                stab_insert(scope_table, tmp_token.data.string.string); //! vlozim to table
            }
            codegen_assign(stab_dict_get(dict, tmp_token.data.string.string), false);
            if ((actual_token.type == TOKEN_ASSIGN) || (actual_token.type == TOKEN_COLON))
            {
                // kontrola ci bola deklarovane
                if (stab_dict_lookup(dict,tmp_token.data.string.string) == false) return DEFINITION_ERROR;
                node_t* tmp_id; //! potrebuje ziskat typ ID
                tmp_id = stab_dict_get(dict,tmp_token.data.string.string);
                switch(tmp_id->type)
                {
                    case TOKEN_INT :
                        return_typ[index_return] = 'i';
                        break;

                    case TOKEN_FLOAT :
                        return_typ[index_return] = 'f';
                        break;

                    case TOKEN_STRING :    
                        return_typ[index_return] = 's';
                        break;
                }
                index_return ++;
            }
            eaten_token = true; //! <dec_and_def> cita token na zaciatku
            result = dec_and_def(dict, scope_table);                                                // -> <dec_and_def>
            if (result != SYNTAX_OK) return result;

            get_new_token(false);
            if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;
           
            if (actual_token.type != TOKEN_NEW_LINE)                                                // -> eol 
            {
                return SYNTAX_ERROR;
            }
            return statement(dict, scope_table);                                                    // -> <statement>
        //
            
        //  <statement> -> _ <dec_and_def> eol <statement>
        case TOKEN_VAR_UNDERSCORE :                                                                 // -> _
            tmp_token = actual_token;
            is_underscore = true;
            is_another_underscore = false;
            index_return = 0;
            return_typ[index_return] = '_';
            index_return ++;
            result = dec_and_def(dict, scope_table);                                                // -> <dec_and_def>
            if (result != SYNTAX_OK) return result;

            get_new_token(false);
            if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;

            if (actual_token.type != TOKEN_NEW_LINE)                                                // -> eol 
            {
                return SYNTAX_ERROR;
            }
            return statement(dict, scope_table);                                                    // -> <statement>
        //    
         default:
            break;
    }

    //  <statement> -> eps
    eaten_token = true;
    return SYNTAX_OK;
}




/* ****************
 * <dec_and_def>
 * ***************/
int dec_and_def(stab_dict_t dict, root_t scope_table)
{
    int result;
    in_define = false;

    get_new_token(false);
    if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;

    switch(actual_token.type)
    {
        // <dec_and_def> -> := <assign>
        case TOKEN_DEFINE :                                                                         // -> :=
            in_define = true; //!na osetrenie <function>
       
            result = assign(dict, scope_table);                                                     // -> <assign>
            if (result != SYNTAX_OK) return result;
            return SYNTAX_OK;
        //    


        //  <dec_and_def> <next_id> = <assign>
        case TOKEN_COLON :                                                                          // -> <next_id>
        case TOKEN_ASSIGN : 
            in_define = false;
            in_assign = true;
  
            result = next_id(dict, scope_table);
            if (result != SYNTAX_OK) return result;
            
            if (actual_token.type != TOKEN_ASSIGN)                                                  // -> =
            {
                return SYNTAX_ERROR;
            }
            result = assign(dict ,scope_table);                                                     // -> <assign>
            if (result != SYNTAX_OK) return result;
            in_assign = false;
            return SYNTAX_OK;
        //    
         default:
            break;
    }

    return SYNTAX_ERROR;        //! asi tu ma byt
}




/* ****************
 * <for_init>
 * ***************/
int for_init(stab_dict_t dict, root_t scope_table)
{
    int result;
    //in_define = false; !// mozno zle
    for_init_empty = false;

    get_new_token(false);
    if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;

    switch(actual_token.type)
    {
        //  <for_init> -> id <dec_and_def> 
        case TOKEN_VAR :                                                                            // -> id
            tmp_token = actual_token;
            get_new_token(false);
            if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;

            if (actual_token.type == TOKEN_DEFINE)
            {
                // kontrola ci sa nejdena o redifiniciu
                if (stab_lookup(scope_table, tmp_token.data.string.string)) return DEFINITION_ERROR;

                stab_insert(scope_table, tmp_token.data.string.string); //! vlozim to table
            }
            codegen_assign(stab_dict_get(dict, tmp_token.data.string.string), false);
            if ((actual_token.type == TOKEN_ASSIGN) || (actual_token.type == TOKEN_COLON))
            {    
                // kontrola ci bola deklarovane
                if (stab_dict_lookup(dict,tmp_token.data.string.string) == false) return DEFINITION_ERROR;
            }
            eaten_token = true; //! <dec_and_def> cita token na zaciatku
            
            result = dec_and_def(dict, scope_table);                                                // -> <dec_and_def>
            if (result != SYNTAX_OK) return result;
            
            return SYNTAX_OK;
        //    
        default:
            break;
    }

    //  <for_init> -> eps
    for_init_empty = true;
    return SYNTAX_OK; // mozno eaten_token
}




/* ****************
 * <assign>
 * ***************/
int assign(stab_dict_t dict, root_t scope_table)
{
    int result;

    get_new_token(false);
    if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;

    if (actual_token.type >= TOKEN_PRINT && actual_token.type <= TOKEN_CHR){
      if (in_define) return SYNTAX_ERROR; 

      result = function(dict, scope_table);                                                   // -> <function>
      if (result != SYNTAX_OK) 
         return result;
      return SYNTAX_OK;
    }

    /* switch(actual_token.type) */
    /* { */
    /*     //  <assign> -> <function> */
    /*     case TOKEN_VAR_FUNC ... TOKEN_CHR : */
    /*         // kontrola ci nie sme v "define" */
    /*         if (in_define) return SYNTAX_ERROR;  */
    /*  */
    /*         result = function(dict, scope_table);                                                   // -> <function> */
    /*         if (result != SYNTAX_OK) return result;\ */
    /*         return SYNTAX_OK; */
    /*     //     */
    /*      default: */
    /*         break; */
    /* } */
    
    //  <assign> -> <expression>
    eaten_token = true;     //! mozno zle 
    //if (is_another_underscore) return SYNTAX_ERROR;

    if (in_define)
    {
        where = DECLARATION;
    }
    else
    {
        /*
        if (is_underscore)
        {
            where = BOOL_FALSE;
        }
        */  
        if (multiple)
        {
            where = IN_MULTIPE;
            //multiple = false;
        }
        else
            where = ASSIGN;

        is_underscore = false;    
    }
    
    if(multiple)
    {
        result = expr(dict,scope_table);
		g_string_instr = false;
        codegen_assign(NULL, true);
        eaten_token = true;
        multiple = false;
    }
    else 
    {
        result = expression(dict,scope_table,tmp_token);  
		g_string_instr = false;
        codegen_assign(NULL, true);
      /* codegen_end_expr(stab_dict_get(dict, tmp_token.data.string.string)); */
    }
    
    // -> <expression>
    if (result != SYNTAX_OK) return result;
    
    return SYNTAX_OK;
}




/* ****************
 * <function>
 * ***************/
int function(stab_dict_t dict, root_t scope_table)
{
    int result;

    //if (strcmp(actual_token.data.string.string,"print")) 
    //{
        strcpy(key, actual_token.data.string.string);
    //}
    node_t* tmp_func_node;
    
    //! mozno az na koci kontrola
    if (!stab_lookup(dict->global->root,key)) 
    {
        stab_insert(dict->global->root,key);
        tmp_func_node = stab_get(dict->global->root,key);
        tmp_func_node->declared = false; // vlozi funkciu do tabulky a potom neskorsie porovnam
    }
    else
    {
        tmp_func_node = stab_get(dict->global->root,key);   
    }

    switch(actual_token.type)
    {
        case TOKEN_VAR_FUNC :                   //  <function> -> id_func
        case TOKEN_INPUT_INT :                  //  <function> -> inputi
        case TOKEN_INPUT_FLOAT :                //  <function> -> inputs
        case TOKEN_INPUT_STRING :               //  <function> -> inputs                
        case TOKEN_INT2FLOAT :                  //  <function> -> int2float
        case TOKEN_FLOAT2INT :                  //  <function> -> float2int
        case TOKEN_LEN :                        //  <function> -> len
        case TOKEN_SUBSTR :                     //  <function> -> substr
        case TOKEN_ORD :                        //  <function> -> ord
        case TOKEN_CHR :                        //  <function> -> chr
            
            if (actual_token.type != TOKEN_VAR_FUNC)
				if (actual_token.type > TOKEN_VAR_FUNC && actual_token.type < TOKEN_INT2FLOAT)
					codegen_add_include(STRCMP);
                  codegen_add_include(TOKEN_TO_INCLUDE(actual_token.type));
            /* if (actual_token.type >= TOKEN_INPUT_INT && actual_token.type <= TOKEN_CHR) */
            // kontrola iba pokial bola funkcia definovana pred mainom
            if (tmp_func_node->declared)
            {
                if (in_assign)
                {   
                    if (tmp_func_node->return_count != index_return) return RETURN_ERROR;

                    // kontrola return type
                    for (int i = 0; i < index_return; i++)
                    {
                        if (return_typ[i] != '_')
                        {
                            if (tmp_func_node->return_types[i] != return_typ[i]) return RETURN_ERROR;
                        }
                    }
                }
                else
                {
                    if (tmp_func_node->return_count != index_return) return RETURN_ERROR;
                }
                   
            }
            else // funkcia deklarovana za mainom 
            {
                if (in_assign)
                {
                    tmp_func_node->return_count = index_return; // ulozime pocet navratovych hodnot

                    for (int i = 0; i < index_return; i++)
                    {
                        tmp_func_node->return_types[i] = return_typ[i];
                    }
                }
            } 
            
            get_new_token(false);
            if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;
            
            if (actual_token.type != TOKEN_LEFT_ROUND)                                              // -> (
            {
                return SYNTAX_ERROR;
            }

            result = arg(dict, scope_table, tmp_func_node);                                         // -> <arg>
            if (result != SYNTAX_OK) return result;
            // TODO int2float, float2int and strlen could be printed right into
            // the code
            printf("CALL %s\n", tmp_func_node->key); 
            codegen_assign(NULL, true);
            // kontrola poctu parametrov iba ak je funkcia declared
            if(tmp_func_node->declared)
                if (tmp_func_node->param_count != index_param) return PARAM_CNT_ERROR;

            // <arg> nacitalo newToken
            if (actual_token.type != TOKEN_RIGHT_ROUND)                                             // -> )
            {
                return SYNTAX_ERROR;
            }
            return SYNTAX_OK;
        //
        

        //  <function> -> print ( <expr> )
        case TOKEN_PRINT :                                                                          // -> print
            get_new_token(false);
			if (in_assign) return PARAM_CNT_ERROR;
            if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;
            
            if (actual_token.type != TOKEN_LEFT_ROUND)                                              // -> (
            {
                return SYNTAX_ERROR;
            }
            where = IN_PRINT;
            result = expr(dict, scope_table);  
            where = BOOL_FALSE;                                                                     // -> <expr>
            if (result != SYNTAX_OK) return result;

            if (actual_token.type != TOKEN_RIGHT_ROUND)                                             // -> )
            {
                return SYNTAX_ERROR;
            }
            return SYNTAX_OK;
        //
        default:
            break;
    }
    return SYNTAX_OK; // NOTE this is impossible to happen
}



/* ****************
 * <expr>
 * ***************/
int expr(stab_dict_t dict, root_t scope_table)
{
    int result;

    get_new_token(false);
    if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;
   
    //  <expr> -> <expression> <expr_n>
    if(!multiple && where != IN_PRINT)
    {
        where = BOOL_FALSE;
    }
    // token som predal expression()
    result = expression(dict,scope_table,tmp_token);                                                // -> <expression>
    if (result != SYNTAX_OK) return result;

    result = expr_n(dict,scope_table);                                                              // -> <expr_n>
    if (result != SYNTAX_OK) return result;

    return SYNTAX_OK;
}



/* ****************
 * <exp_n>
 * ***************/
int expr_n(stab_dict_t dict, root_t scope_table)
{
    int result;

    switch(actual_token.type)
    {   
        //  <expr_n> -> , <expression> <expr_n>
        case TOKEN_COLON :                                                                          // -> ,
            get_new_token(false);
            if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;
            
            // token predam expression
            if(!multiple && where != IN_PRINT){
                where = BOOL_FALSE;
            }
   
            result = expression(dict,scope_table,tmp_token);                                        // -> <expression>
            if (result != SYNTAX_OK) return result;
          
            return expr_n(dict,scope_table);                                                        // -> <expr_n>
        //
        default:
            break;
    }

    //  <expr_n> -> eps
    return SYNTAX_OK;

}




/* ****************
 * <next_id>
 * ***************/
int next_id(stab_dict_t dict, root_t scope_table)
{
    switch(actual_token.type)
    {
        // <next_id> -> , id <next_id>
        case TOKEN_COLON :                                                                          // -> ,
            get_new_token(false);
            multiple = true;
            
            if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;
        
            if ((actual_token.type != TOKEN_VAR) && (actual_token.type != TOKEN_VAR_UNDERSCORE))    // -> id || _
            {
                return SYNTAX_ERROR;
            }
            if (actual_token.type != TOKEN_VAR_UNDERSCORE)
               codegen_assign(stab_dict_get(dict, actual_token.data.string.string), false);
            else
               codegen_assign(error_node, false);
            
            //! pokial nacitalo "_" neriesim ci bolo deklarovane
            if (actual_token.type != TOKEN_VAR_UNDERSCORE)
            {   
                is_another_underscore = false;
                
                //! kontrola ci bolo deklarovane
                if (stab_dict_lookup(dict,actual_token.data.string.string) == false) return DEFINITION_ERROR;
                
                node_t* tmp_id; //! potrebuje ziskat typ ID
                tmp_id = stab_dict_get(dict,actual_token.data.string.string);
                switch(tmp_id->type)
                {
                    case TOKEN_INT :
                        return_typ[index_return] = 'i';
                        break;

                    case TOKEN_FLOAT :
                        return_typ[index_return] = 'f';
                        break;

                    case TOKEN_STRING :    
                        return_typ[index_return] = 's';
                        break;
                }
                index_return ++;
            }
            else 
            {
                return_typ[index_return] = '_';
                index_return ++;
                is_another_underscore = true;
            }
            get_new_token(false);
            if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;

            return next_id(dict, scope_table);                                                      // -> <next_id>
        //


        //  <next_id> -> eps
        case TOKEN_ASSIGN :
            return SYNTAX_OK;
         default:
            break;

    }
    return SYNTAX_ERROR;    //! asi tu ma byt
}




/* ****************
 * <arg>
 * ***************/
int arg(stab_dict_t dict, root_t scope_table,node_t* tmp_func_node)
{
    int result;
    index_param = 0; //!

    get_new_token(true); //! dal som na tru
    if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;
            
    switch(actual_token.type)
    {
        case TOKEN_VAR :
        case TOKEN_INT :
        case TOKEN_FLOAT :
        case TOKEN_STRING :

        //  <arg> -> <value> <arg_n>
        result = value(dict, tmp_func_node);                                           // -> <value>
        if (result != SYNTAX_OK) return result;

        result = arg_n(dict, scope_table, tmp_func_node);                                           // -> <arg_n>
        if (result != SYNTAX_OK) return result;
        
        return SYNTAX_OK;
        default:
           break;
    }

    //  <arg> -> eps
    return SYNTAX_OK;
}




/* ****************
 * <arg_n>
 * ***************/
int arg_n(stab_dict_t dict, root_t scope_table,node_t* tmp_func_node)
{
    int result;

    get_new_token(false);
    if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;
   
    switch(actual_token.type)
    {
        //  <arg_n> -> , <value> <arg_n>
        case TOKEN_COLON :                                                                          // -> ,
         
            get_new_token(true); //!dal som tu true
            if (actual_token.type == TOKEN_UNDEFINED) return LEXICAL_ERROR;
          
            // novy token pre <value>
            result = value(dict, tmp_func_node);                                       // -> <value>
            if (result != SYNTAX_OK) return result;
     
            return arg_n(dict, scope_table, tmp_func_node);                                         // -> <argn_n>
         default:
            break;
    }

    // <arg_n> -> eps
    return SYNTAX_OK;
}




/* ****************
 * <value>
 * ***************/
int value(stab_dict_t dict, node_t* tmp_func_node)
{
    // iba pokial je definova
    if (tmp_func_node->declared)
    {   
        // kontrola ci necital dalsi arg -> ked nema
        if (tmp_func_node->param_count == index_param)
        {
            return PARAM_CNT_ERROR;
        }
    }

    switch(actual_token.type)
    {
        //  <value> -> id
        case TOKEN_VAR :                                                                            
            // kontrola ci bolo deklarovane
            if (stab_dict_lookup(dict,actual_token.data.string.string) == false) return DEFINITION_ERROR;
            
            node_t* tmp_id; //! potrebuje ziskat typ ID
            tmp_id = stab_dict_get(dict,actual_token.data.string.string);
            
            printf("PUSHS %s\n", tmp_id->ir);
            switch(tmp_id->type)
            {
                case TOKEN_INT :
                    if (tmp_func_node->declared)
                    {
                        if (tmp_func_node->params[index_param] != 'i') return PARAM_CNT_ERROR;
                        index_param ++;
                    }
                    else
                    {
                        tmp_func_node->params[index_param] = 'i';
                        
                        index_param ++;
                        tmp_func_node->param_count = index_param;
                    }
                    return SYNTAX_OK;

                case TOKEN_FLOAT :
                    if (tmp_func_node->declared)
                    {
                        if (tmp_func_node->params[index_param] != 'f') return PARAM_CNT_ERROR;
                        index_param ++;
                    }
                    else
                    {
                        tmp_func_node->params[index_param] = 'f';

                        index_param ++;
                        tmp_func_node->param_count = index_param;
                    }
                    return SYNTAX_OK;

                case TOKEN_STRING :
                    if (tmp_func_node->declared)
                    {
                        if (tmp_func_node->params[index_param] != 's') return PARAM_CNT_ERROR;
                        index_param ++;
                    }
                    else
                    {
                        tmp_func_node->params[index_param] = 's';
                     
                        index_param ++;
                        tmp_func_node->param_count = index_param;
                    }
                    return SYNTAX_OK;         
            }
        //
        

        //  <value> -> int_value
        case TOKEN_INT :                                                                            
            if (tmp_func_node->declared)    
            {
                if (tmp_func_node->params[index_param] != 'i') return PARAM_CNT_ERROR;

                index_param ++; 
            }  
            else
            {
                tmp_func_node->params[index_param] = 'i';

                index_param ++;
                tmp_func_node->param_count = index_param;
            }
            codegen_expr(INT, actual_token.data.integer);
            return SYNTAX_OK;
        //

        //  <value> -> float_value
        case TOKEN_FLOAT :                                                                          
            if (tmp_func_node->declared)
            {
                if (tmp_func_node->params[index_param] != 'f') return PARAM_CNT_ERROR;
                index_param ++; 
            }   
            else
            {
                tmp_func_node->params[index_param] = 'f';

                index_param ++;
                tmp_func_node->param_count = index_param;

            }
            codegen_expr(FLOAT, actual_token.data.integer);
            return SYNTAX_OK;
        //


        //  <Value> -> string_value
        case TOKEN_STRING :                                                                         
            if (tmp_func_node->declared)
            {
                if (tmp_func_node->params[index_param] != 's') return PARAM_CNT_ERROR;
                index_param ++;
            }
            else
            {
                tmp_func_node->params[index_param] = 's';

                index_param ++;
                tmp_func_node->param_count = index_param;
            }
            codegen_expr(STRING, actual_token.data.integer);
            return SYNTAX_OK;
        //    
         default:
            break;
    }
    return SYNTAX_ERROR; 
}


/* **************************************
 * Initialization of built-in functions
 * *************************************/
void functions_init (stab_dict_t dict)
{
    node_t tmp ;

    // function inputs() (string,int)
    tmp.declared = true;
    tmp.param_count = 0;
    tmp.return_count = 2;
    tmp.return_types[0] = 's';
    tmp.return_types[1] = 'i';
    stab_function_set(dict->global->root, "inputs",&tmp);

    // function inputi() (int,int)
    tmp.declared = true;
    tmp.param_count = 0; 
    tmp.return_count = 2;
    tmp.return_types[0] = 'i';
    tmp.return_types[1] = 'i';
    stab_function_set(dict->global->root, "inputi",&tmp);

    // function inputf() (float64,int)
    tmp.declared = true;
    tmp.param_count = 0; 
    tmp.return_count = 2;
    tmp.return_types[0] = 'f';
    tmp.return_types[1] = 'i';
    stab_function_set(dict->global->root, "inputf",&tmp);

    // function int2float (int) (float64)
    tmp.declared = true;
    tmp.param_count = 1; 
    tmp.params[0] ='i'; 
    tmp.return_count = 1;
    tmp.return_types[0] = 'f';
    stab_function_set(dict->global->root, "int2float",&tmp);

    // function float2int (float64) (int)
    tmp.declared = true;
    tmp.param_count = 1; 
    tmp.params[0] ='f'; 
    tmp.return_count = 1;
    tmp.return_types[0] = 'i';
    stab_function_set(dict->global->root, "float2int",&tmp);

    // function len (string) (int)
    tmp.declared = true;
    tmp.param_count = 1; 
    tmp.params[0] ='s'; 
    tmp.return_count = 1;
    tmp.return_types[0] = 'i';
    stab_function_set(dict->global->root, "len",&tmp);

    // function substr (string,int,int) (string,int)
    tmp.declared = true;
    tmp.param_count = 3; 
    tmp.params[0] ='s'; 
    tmp.params[1] ='i';
    tmp.params[2] ='i';
    tmp.return_count = 2;
    tmp.return_types[0] = 's';
    tmp.return_types[1] = 'i';
    stab_function_set(dict->global->root, "substr",&tmp);

    // function ord (string,int) (int,int)
    tmp.declared = true;
    tmp.param_count = 2; 
    tmp.params[0] ='s';
    tmp.params[1] ='i'; 
    tmp.return_count = 2;
    tmp.return_types[0] = 'i';
    tmp.return_types[1] = 'i';
    stab_function_set(dict->global->root, "ord",&tmp);

    // function chr (int) (string,int)
    tmp.declared = true;
    tmp.param_count = 1; 
    tmp.params[0] ='i'; 
    tmp.return_count = 2;
    tmp.return_types[0] = 's';
    tmp.return_types[1] = 'i';
    stab_function_set(dict->global->root, "chr",&tmp);

    
    // function print ()
    tmp.declared = true;
    stab_function_set(dict->global->root, "print",&tmp);
    
}


/* ****************
 * parser
 * ***************/
int parser()
{
    int result;
    actual_error_node.ir = "GF@ERR";
    error_node = &actual_error_node;

    dyn_str_list_init();
    stab_dict_t dict = stab_dict_init();
    stab_root_init(dict);
    
    // Initialization of built-in functions
    functions_init(dict);
    // Dealloc of global symtable
   


    result = prolog(dict);
    if (result != SYNTAX_OK){
       dispose_dyn_str_list(list_of_dyn_strings);
        stab_destroy(dict->global->root);
         free(dict->head);
         free(dict);
        return result;
    }

    
    if ((stab_are_functions_defined(dict->global->root->node)) == false)
    {
       dispose_dyn_str_list(list_of_dyn_strings);
         stab_destroy(dict->global->root);
         free(dict->head);
         free(dict);
        return DEFINITION_ERROR;
    }
    dispose_dyn_str_list(list_of_dyn_strings);
    stab_destroy(dict->global->root);
    free(dict->head);
    free(dict);
    return SYNTAX_OK;
}
