/**
 * expression.c - Semantical Analysis of code
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

#include "expression.h"

/* https://www.vutbr.cz/www_base/zav_prace_soubor_verejne.php?file_id=117133 - popis cyklu pomocou syntaktickej precendencnej analyzy bod 4.2.1 */ // this is web which help me the most


int where;
bool token_was_war;
static bool in_brackets = false;

#define STACK_EMPTY 0
#define LENGTH_ARRAY_OF_VARIABLES 10

/* prec table  */
int global_counter_for = 0;                     // this is global variable whoch i use for counting how many variables i assigned or returned in function "a,b = 1,3"

int prec_table[7][7] =                                    
{
//	|+- | */| r |   ( |   ) |   i |   $ |
	
    { M , L , M   , L   , M   , L   , M }, // +-
	{ M , M , M   , L   , M   , L   , M }, // */
	{ L , L , ERR , L   , M   , L   , M }, //  <> <= < >= > /r
	{ L , L , L   , L   , E   , L   , M }, // (
	{ M , M , M   , ERR , M   , ERR , M }, // )
	{ M , M , M   , ERR , M   , ERR , M }, // i 
	{ L , L , L   , L   , ERR , L   , DONE } // $
};

int Prec(Token token) 
{ 
    switch (token.type) 
    { 
    case TOKEN_PLUS: 
    case TOKEN_MINUS: 
        return PLUS_MINUS; 
  
    case TOKEN_MULTIPLY: 
    case TOKEN_DIVISION: 
        return MULT_DIV; 
  
    case TOKEN_GREATER:
    case TOKEN_GREATER_EQUAL:
    case TOKEN_LESS:
    case TOKEN_LESS_EQUAL:
    case TOKEN_EQUAL:
    case TOKEN_NOT_EQUAL:
        return RELATIONS; 

    case TOKEN_LEFT_ROUND:
        return LEFT_BR;

    case TOKEN_RIGHT_ROUND:
        return RIGHT_BR;

    case TOKEN_VAR:
    case TOKEN_FLOAT:
    case TOKEN_FLOAT_EXP:
    case TOKEN_INT:
    case TOKEN_INT_EXP:
    case TOKEN_STRING:
        return IDENTIFIER;

    case TOKEN_DOLLAR:
    case TOKEN_EOF:
    case TOKEN_NEW_LINE:
    case TOKEN_NON_TERM:
    case TOKEN_SHIFT:
    case TOKEN_LEFT_CURLY:
    case TOKEN_SEMICOLON:
    case TOKEN_COLON:
        return DOLAR;       
                                    
    default:
        return NON_SYMBOL_IN_EXPRESSION;        // this returns always error
    } 
   
} 



bool push( list **stack, Token *sym, Token skuska)                          // just copy whole TOKEN to the pushed TOKEN
{
    list *new_element = malloc( sizeof( list ) );
    
    if (skuska.type == TOKEN_NON_TERM) {                                    // i use this, because push is called in function "return_semantic" and
        new_element->symbol.type = TOKEN_NON_TERM;                          // is pushing there always TOKEN_NONTERM but in function EXPRESSION 
    } else {                                                                // it is pushing TOKEN_VAR or TOKEN_INT || TOKEN_FLOAT ....
        
        new_element->symbol.type = (*sym).type;
    }

	if (token_was_war) {
		dyn_str_cpy(&(new_element->symbol.data.string),&((*sym).data.string)); 
	} else {
		if((*sym).type == TOKEN_INT)
			 new_element->symbol.data.integer = (*sym).data.integer;
		else if((*sym).type == TOKEN_FLOAT || (*sym).type == TOKEN_INT_EXP || (*sym).type == TOKEN_FLOAT_EXP)
			new_element->symbol.data.dbl = (*sym).data.dbl;
		else if((*sym).type == TOKEN_STRING && in_brackets == false){
			dyn_str_cpy(&(new_element->symbol.data.string),&((*sym).data.string)); 
		}
	}

	new_element->type = (*sym).type;

	new_element->next = *stack;
	*stack = new_element;

    return true;
}


void skip_new_line() {                                
                                                            
    actual_token = getToken(); 

                while (actual_token.type == TOKEN_NEW_LINE) {
                   actual_token = getToken(); 
                }

}

Token pop(stack *mystack) {
     
    if (*mystack == NULL) {
        exit(1); //internal error, it is not going to happen
    }

    list *tmp;
    Token top = (*mystack)->symbol;
    tmp = *mystack;
    *mystack = (*mystack)->next;
    free(tmp);
 
    return top;  
}

void destroy(struct list** mystack){
   
   if(*mystack == NULL) {
       return;
   }

   destroy(&((*mystack)->next));
   free(*mystack);
   *mystack = NULL; 
}



Token top_terminal_in_stack(stack *mystack) {   

    Token symbol_dollar;
    symbol_dollar.type = TOKEN_DOLLAR;

    if (*mystack == NULL)  exit(1);
    
    stack tmp = *mystack;

    while (tmp != NULL)
    {
        
         if (Prec(tmp->symbol) < DOLAR)
         {
             return tmp->symbol;
         }

        tmp = tmp->next;
         
    }

    return symbol_dollar;
}



void insert_after_last(stack *mystack) {       // it inserts "<" after last non_term element

    list *new = malloc(sizeof(list));
    new->next = NULL;
    new->symbol.type = TOKEN_SHIFT;
    list *tmp = (*mystack);
    list *one_step_behind = NULL;
    
    while (tmp != NULL) {

         if (Prec(tmp->symbol) != DOLAR || tmp->symbol.type == TOKEN_DOLLAR) {  // when symbol.type is dollar it means  that we are in the last element an have to do something
            
            if (one_step_behind == NULL) {
                
                new->next = (*mystack);
                (*mystack) = new;
                return;

			} else {

                new->next =one_step_behind->next;
			    one_step_behind->next = new;
                return;

			}
        }
            
        one_step_behind = tmp;
        tmp = tmp->next;
         
    }

}

void semantic_return(stack *mystack,Token *value_to_be_pushed,int counter) {        // because i always return numbers i cant just pop stack in the end of function reduce i can but it will be not very nice

    Token temporary;
    
    for (int i = 0; i <= counter; i++)     {     // pop stack from 3 and 1 acording how many terms we have + symbol shift 

        temporary = pop(&(*mystack));
		(void)temporary;
    }

    Token non;
    non.type = TOKEN_NON_TERM;
    
    push(&(*mystack),&(*value_to_be_pushed),non);

}


int semantic (Token first_nonterm, Token operand, Token second_nonterm,int counter, Token *final_non,stack *mystack, stab_dict_t dict) {

    if(counter == 1) {

        

        if (token_was_war) {                            // if token is var i rewrite it on token_int .... so i have to remember if token was var
             //("%s \n",first_nonterm.data.string);
			if (where == IN_PRINT)
				codegen_write(NODE, stab_dict_get(dict, first_nonterm.data.string.string));
			else
				codegen_expr(NODE, stab_dict_get(dict, first_nonterm.data.string.string));
            dyn_str_cpy(&((*final_non).data.string),&(first_nonterm.data.string));
			token_was_war = false;       

        }   else if((*mystack)->type == TOKEN_FLOAT || (*mystack)->type == TOKEN_FLOAT_EXP) {
            //("%f\n",first_nonterm.data.dbl);
			if (where == IN_PRINT)
				codegen_write(FLOAT, first_nonterm.data.dbl);
			else
				codegen_expr(FLOAT, first_nonterm.data.dbl);
            (*final_non).data.dbl = first_nonterm.data.dbl;
        } else if ((*mystack)->type == TOKEN_INT) {
			if (where == IN_PRINT)
				codegen_write(INT, first_nonterm.data.integer);
			else
				codegen_expr(INT, first_nonterm.data.integer);
            //("%lld\n",first_nonterm.data.integer);
            (*final_non).data.integer = first_nonterm.data.integer;
           
        } else {
             //("%s\n",first_nonterm.data.string);
			g_string_instr = true;
            dyn_str_cpy(&((*final_non).data.string),&(first_nonterm.data.string));
			if (where == IN_PRINT)
				codegen_write(STRING, first_nonterm.data.string.string);
			else
				codegen_expr(STRING, first_nonterm.data.string.string);
        }
        

        (*final_non).type = first_nonterm.type;
        semantic_return(&(*mystack),&(*final_non),counter);
        return OK;

    } else if (counter == 3) {

        
        
    
        // rule E -> (E)
        if(first_nonterm.type == TOKEN_LEFT_ROUND && second_nonterm.type == TOKEN_RIGHT_ROUND && operand.type == TOKEN_NON_TERM) {
            
            if (token_was_war) {
           
            //("( %s )\n",operand.data.string);
            token_was_war = false;

            } else  if((*mystack)->next->type == TOKEN_FLOAT || (*mystack)->next->type  == TOKEN_FLOAT_EXP) {     // NEBUDES MOZNO ANI POSIELAT

            //("( %f )\n",operand.data.dbl);

            } else if ((*mystack)->next->type  == TOKEN_INT || (*mystack)->next->type  == TOKEN_INT_EXP) {
                
            //("( %lld )\n",operand.data.integer);

            } else {

            //("( %s )\n",operand.data.string);

            }
            
   
            
			in_brackets = true;
            (*final_non).type = (*mystack)->next->type;
            semantic_return(&(*mystack),&(*final_non),counter);
			in_brackets = false;
            return OK;
           
        }

       
        
        if( operand.type == TOKEN_DIVISION && (second_nonterm.data.integer ==  0 && second_nonterm.data.dbl ==  0.0)) {
         
            if( (*mystack)->next->next->type != TOKEN_STRING) {
                
                //("%d semantic error ------> (zero divison)\n",9);
                return ZERO_DIV_ERROR;
            }

        }

        

        if (first_nonterm.type == TOKEN_NON_TERM  && second_nonterm.type == TOKEN_NON_TERM)
		{
			switch (operand.type)
			{
			// rule E -> E + E
			case TOKEN_PLUS:
                
                if ((*mystack)->type == (*mystack)->next->next->type) {

                    if (token_was_war) {
                         
                        //("%s + %s\n",first_nonterm.data.string,second_nonterm.data.string);
                    token_was_war = false;

                    } else  if((*mystack)->type == TOKEN_FLOAT || (*mystack)->type  == TOKEN_FLOAT_EXP) {     // NEBUDES MOZNO ANI POSIELAT

                        //("%f + %f\n",first_nonterm.data.dbl,second_nonterm.data.dbl);

                    } else if ((*mystack)->type  == TOKEN_INT || (*mystack)->type  == TOKEN_INT_EXP) {
                         //("som tu");
                        //("%lld + %lld\n",first_nonterm.data.integer,second_nonterm.data.integer);

                    } else {

                        //("%s + %s\n",first_nonterm.data.string,second_nonterm.data.string);

					}
					codegen_instr(ADD);
                    (*final_non).type = (*mystack)->type;
                    semantic_return(&(*mystack),&(*final_non),counter);
				    return OK;
                
                }
               
                //("%d semantic error ------> (bad types)\n",5);
                return TYPE_COMPABILITY_ERROR;

			// rule E -> E - E
			case TOKEN_MINUS:
                
                if ((*mystack)->type == (*mystack)->next->next->type)  {

                    if ((*mystack)->type == TOKEN_STRING || (*mystack)->next->next->type == TOKEN_STRING) {
                        
                        //("%d semantic error ------> (string bad operation)\n",7);
                        return TYPE_COMPABILITY_ERROR;  
                    }
                    
                    if (token_was_war) {
                        
                        //("%s - %s\n",first_nonterm.data.string,second_nonterm.data.string);
                    token_was_war = false;

                    } else  if((*mystack)->type == TOKEN_FLOAT || (*mystack)->type  == TOKEN_FLOAT_EXP) {     // NEBUDES MOZNO ANI POSIELAT

                        //("%f - %f\n",first_nonterm.data.dbl,second_nonterm.data.dbl);

                    } else if ((*mystack)->type  == TOKEN_INT || (*mystack)->type  == TOKEN_INT_EXP) {
                       
                        //("%lld - %lld\n",first_nonterm.data.integer,second_nonterm.data.integer);

                    } else {
                        //("%s - %s\n",first_nonterm.data.string,second_nonterm.data.string);

                    }

                   	codegen_instr(SUB);
                    (*final_non).type = (*mystack)->type;
                    semantic_return(&(*mystack),&(*final_non),counter);
				    return OK;
                }
                
                //("%d semantic error ------> (bad types)\n",5);
                return TYPE_COMPABILITY_ERROR;

			// rule E -> E * E
			case TOKEN_MULTIPLY:

                if ((*mystack)->type == (*mystack)->next->next->type)  {

                    if ((*mystack)->type == TOKEN_STRING || (*mystack)->next->next->type == TOKEN_STRING) {
                        
                        //("%d semantic error ------> (string bad operation)\n",7);
                        return TYPE_COMPABILITY_ERROR;   
                    }

                    if (token_was_war) {
           
                        //("%s * %s\n",first_nonterm.data.string,second_nonterm.data.string);
                    token_was_war = false;

                    } else  if((*mystack)->type == TOKEN_FLOAT || (*mystack)->type  == TOKEN_FLOAT_EXP) {     // NEBUDES MOZNO ANI POSIELAT

                        //("%f * %f\n",first_nonterm.data.dbl,second_nonterm.data.dbl);

                    } else if ((*mystack)->type  == TOKEN_INT || (*mystack)->type  == TOKEN_INT_EXP) {
                
                        //("%lld * %lld\n",first_nonterm.data.integer,second_nonterm.data.integer);

                    } else {

                        //("%s * %s\n",first_nonterm.data.string,second_nonterm.data.string);

                    }
                    
                
                   	codegen_instr(MUL);
                    (*final_non).type = (*mystack)->type;
                    semantic_return(&(*mystack),&(*final_non),counter);
				    return OK;
                }
                
                //("%d semantic error ------> (bad types)\n",5);
                return TYPE_COMPABILITY_ERROR; 

			// rule E -> E / E
			case TOKEN_DIVISION:
                if ((*mystack)->type == (*mystack)->next->next->type)  {

                    if ((*mystack)->type == TOKEN_STRING || (*mystack)->next->next->type == TOKEN_STRING) {
                        
                        //("%d semantic error ------> (string bad operation)\n",7);
                        return TYPE_COMPABILITY_ERROR;   
                    }

                    
                    if (token_was_war) {
           
                    //("%s / %s\n",first_nonterm.data.string,second_nonterm.data.string);
                    token_was_war = false;

                    } else  if((*mystack)->type == TOKEN_FLOAT || (*mystack)->type  == TOKEN_FLOAT_EXP) {     // NEBUDES MOZNO ANI POSIELAT

						codegen_instr(DIV);
                        //("%f  /%f\n",first_nonterm.data.dbl,second_nonterm.data.dbl);

                    } else if ((*mystack)->type  == TOKEN_INT || (*mystack)->type  == TOKEN_INT_EXP) {
                
						codegen_instr(IDIV);
                        //("%lld / %lld\n",first_nonterm.data.integer,second_nonterm.data.integer);

                    } else {

                        //("%s / %s\n",first_nonterm.data.string,second_nonterm.data.string);

                    }

                    (*final_non).type = (*mystack)->type;
                    semantic_return(&(*mystack),&(*final_non),counter);
				    return OK;
                } else {
                    
                    
                    //("%d semantic error ------> (bad types)\n",5);
                    return TYPE_COMPABILITY_ERROR;    

                }
                
    		// rule E -> E == E
			case TOKEN_EQUAL:
				
                if ((*mystack)->type == (*mystack)->next->next->type )  {

                    if (token_was_war) {
           
                    //("%s == %s\n",first_nonterm.data.string,second_nonterm.data.string);
                    token_was_war = false;

                    } else  if((*mystack)->type == TOKEN_FLOAT || (*mystack)->type  == TOKEN_FLOAT_EXP) {     // NEBUDES MOZNO ANI POSIELAT

                        //("%f  == %f\n",first_nonterm.data.dbl,second_nonterm.data.dbl);

                    } else if ((*mystack)->type  == TOKEN_INT || (*mystack)->type  == TOKEN_INT_EXP) {
                
                        //("%lld == %lld\n",first_nonterm.data.integer,second_nonterm.data.integer);

                    } else {

                        //("%s == %s\n",first_nonterm.data.string,second_nonterm.data.string);

                    }
        

					 codegen_logic(EQUAL);
                    (*final_non).type = (*mystack)->type;
                    semantic_return(&(*mystack),&(*final_non),counter);
				    return OK;
                } else {

                    
                    
                    
                    //("%d semantic error ------> (bad types)\n",5);
                    return TYPE_COMPABILITY_ERROR;    

                }

			// rule E -> E != E
			case TOKEN_NOT_EQUAL:
				
                if ((*mystack)->type == (*mystack)->next->next->type)   {

                    if (token_was_war) {
           
                    //("%s != %s\n",first_nonterm.data.string,second_nonterm.data.string);
                    token_was_war = false;

                    } else  if((*mystack)->type == TOKEN_FLOAT || (*mystack)->type  == TOKEN_FLOAT_EXP) {     // NEBUDES MOZNO ANI POSIELAT

                        //("%f  != %f\n",first_nonterm.data.dbl,second_nonterm.data.dbl);

                    } else if ((*mystack)->type  == TOKEN_INT || (*mystack)->type  == TOKEN_INT_EXP) {
                
                        //("%lld != %lld\n",first_nonterm.data.integer,second_nonterm.data.integer);

                    } else {

                        //("%s != %s\n",first_nonterm.data.string,second_nonterm.data.string);

                    }
   
                
					if (g_string_instr)
						codegen_logic(NOT);
					else {
						 codegen_logic(EQUAL);
						 codegen_logic(NOT);
					}
                    (*final_non).type = (*mystack)->type;
                    semantic_return(&(*mystack),&(*final_non),counter);
				    return OK;
                } else {
                    
                    
                    
                    //("%d semantic error ------> (bad types)\n",5);
                    return TYPE_COMPABILITY_ERROR;    

                }

			// rule E -> E <= E
			case TOKEN_LESS_EQUAL:
				
                 if ((*mystack)->type == (*mystack)->next->next->type)     {

                     if (token_was_war) {
           
                    //("%s <= %s\n",first_nonterm.data.string,second_nonterm.data.string);
                    token_was_war = false;

                    } else  if((*mystack)->type == TOKEN_FLOAT || (*mystack)->type  == TOKEN_FLOAT_EXP) {     // token EXP is from last version

                        //("%f  <= %f\n",first_nonterm.data.dbl,second_nonterm.data.dbl);

                    } else if ((*mystack)->type  == TOKEN_INT || (*mystack)->type  == TOKEN_INT_EXP) {
                
                        //("%lld <= %lld\n",first_nonterm.data.integer,second_nonterm.data.integer);

                    } else {

                        //("%s <= %s\n",first_nonterm.data.string,second_nonterm.data.string);

                    }
                    
					 if (g_string_instr)
						 codegen_logic(LESS_EQ);
					 else {
					// Doubled the values on stack
					 printf("POPS LF@TMP2\n");
					 printf("POPS LF@TMP1\n");
					 printf("PUSHS LF@TMP1\n");
					 printf("PUSHS LF@TMP2\n");
					 codegen_logic(LESS);
					 printf("PUSHS LF@TMP1\n");
					 printf("PUSHS LF@TMP2\n");
					 codegen_logic(EQUAL);
					 printf("ORS\n");
					 }
                    (*final_non).type = (*mystack)->type;
                    semantic_return(&(*mystack),&(*final_non),counter);
				    return OK;
                } else {
                    
                    
                    
                    //("%d semantic error ------> (bad types)\n",5);
                    return TYPE_COMPABILITY_ERROR;    

                }

			// rule E -> E < E
			case TOKEN_LESS:
				
                 if ((*mystack)->type == (*mystack)->next->next->type)   {

                     if (token_was_war) {
           
                    //("%s < %s\n",first_nonterm.data.string,second_nonterm.data.string);
                    token_was_war = false;

                    } else  if((*mystack)->type == TOKEN_FLOAT || (*mystack)->type  == TOKEN_FLOAT_EXP) {     // NEBUDES MOZNO ANI POSIELAT

                        //("%f  < %f\n",first_nonterm.data.dbl,second_nonterm.data.dbl);

                    } else if ((*mystack)->type  == TOKEN_INT || (*mystack)->type  == TOKEN_INT_EXP) {
                
                        //("%lld < %lld\n",first_nonterm.data.integer,second_nonterm.data.integer);

                    } else {

                        //("%s < %s\n",first_nonterm.data.string,second_nonterm.data.string);

                    }


					 codegen_logic(LESS);
                    (*final_non).type = (*mystack)->type;
                    semantic_return(&(*mystack),&(*final_non),counter);
				    return OK;
                } else {
                    
                    
                    
                    //("%d semantic error ------> (bad types)\n",5);
                    return TYPE_COMPABILITY_ERROR;  

                }

			// rule E -> E >= E
			case TOKEN_GREATER_EQUAL:
				
                 if ((*mystack)->type == (*mystack)->next->next->type)   {

                     if (token_was_war) {
           
                    //("%s >= %s\n",first_nonterm.data.string,second_nonterm.data.string);
                    token_was_war = false;

                    } else  if((*mystack)->type == TOKEN_FLOAT || (*mystack)->type  == TOKEN_FLOAT_EXP) {     // NEBUDES MOZNO ANI POSIELAT

                        //("%f  >= %f\n",first_nonterm.data.dbl,second_nonterm.data.dbl);

                    } else if ((*mystack)->type  == TOKEN_INT || (*mystack)->type  == TOKEN_INT_EXP) {
                
                        //("%lld >= %lld\n",first_nonterm.data.integer,second_nonterm.data.integer);

                    } else {

                        //("%s >= %s\n",first_nonterm.data.string,second_nonterm.data.string);

                    }

					// Doubled the values on stack
					 if (g_string_instr) {
						codegen_logic(MORE_EQ); 
					 } else {
						 printf("POPS LF@TMP2\n");
						 printf("POPS LF@TMP1\n");
						 printf("PUSHS LF@TMP1\n");
						 printf("PUSHS LF@TMP2\n");
						 codegen_logic(MORE);
						 printf("PUSHS LF@TMP1\n");
						 printf("PUSHS LF@TMP2\n");
						 codegen_logic(EQUAL);
						 printf("ORS\n");
					 }
                    (*final_non).type = (*mystack)->type;
                    semantic_return(&(*mystack),&(*final_non),counter);
				    return OK;
                } else {
                    
                    
                    //("%d semantic error ------> (bad types)\n",5);
                    return TYPE_COMPABILITY_ERROR;   

                }

			// rule E -> E > E
			case TOKEN_GREATER:
				
                 if ((*mystack)->type == (*mystack)->next->next->type)   {

                     if (token_was_war) {
           
                    //("%s > %s\n",first_nonterm.data.string,second_nonterm.data.string);
                    token_was_war = false;

                    } else  if((*mystack)->type == TOKEN_FLOAT || (*mystack)->type  == TOKEN_FLOAT_EXP) {     // NEBUDES MOZNO ANI POSIELAT

                        //("%f  > %f\n",first_nonterm.data.dbl,second_nonterm.data.dbl);

                    } else if ((*mystack)->type  == TOKEN_INT || (*mystack)->type  == TOKEN_INT_EXP) {
                
                        //("%lld > %lld\n",first_nonterm.data.integer,second_nonterm.data.integer);

                    } else {

                        //("%s > %s\n",first_nonterm.data.string,second_nonterm.data.string);

                    }

					 codegen_logic(MORE);
                    (*final_non).type = (*mystack)->type;
                    semantic_return(&(*mystack),&(*final_non),counter);
				    return OK;
                } else {
                    
                     
                    //("%d semantic error ------> (bad types)\n",5);
                    return TYPE_COMPABILITY_ERROR;   

                }
			// invalid operator§ 
			default:
                return SEMANTIC_ERROR;
			}
		}
	return SYNTAX_ERROR;
	}
return SYNTAX_ERROR;
}
				//("%d semantic error ------> \n",7);
			/* } */

        
    /* } */

/* } */

int reduce(stack *mystack, stab_dict_t dict) {
    
    Token tmp1;
    Token tmp2;
    Token tmp3;
    
    int counter = 0;            // counter is used for count how many elements in stack we have to skip as we find an "<"
    int error_num = 0;
    
    
    Token non_terminal;
    

    stack tmp = (*mystack);
   
    while (tmp != NULL)
    {
       
        
       
        if(tmp->symbol.type == TOKEN_SHIFT) {
            break;
        } else {
            counter++;
            tmp = tmp->next;
            
        }


     }

    tmp = (*mystack);

    if(counter == 1 && Prec((*tmp).symbol) == IDENTIFIER ) {

        
        tmp1 = (*mystack)->symbol;
        error_num = semantic(tmp1, tmp1, tmp1,counter,&non_terminal,*(&mystack), dict);
        

    } else if (counter == 3) {

        tmp1 = (*mystack)->next->next->symbol;
        tmp2 = (*mystack)->next->symbol;
        tmp3 = (*mystack)->symbol;
        
        
        
        error_num = semantic(tmp1, tmp2, tmp3,counter,&non_terminal,*(&mystack), dict);  // adding result of operation because of zero division control
        
      
    } else {
  
        //("%d syntax error\n",2);
      
        return SYNTAX_ERROR;

    }

    return error_num;

}

int expression(stab_dict_t dict, root_t scope_table, Token where_store_type) {


	if (where == DECLARATION) {
		codegen_define(stab_dict_get(dict, where_store_type.data.string.string));
	}
    int is_relation = 0 ;
    node_t *where_is_key;                       ///  for find in whit table is key
    node_t *temp;                               
    where_is_key = stab_get(dict->global->root,key);
   
    bool is_colon = false;

    int error_num = 0;
    bool is_bracket_in_print = false;
    Token symbol_dollar;
    symbol_dollar.type = TOKEN_DOLLAR;                  /// this ia will change in .h file

   stack s1 = NULL;

    bool ok_done = false;
    Token top_terminal ;

    bool is_EOL = false;
 
    push(&s1,&symbol_dollar,symbol_dollar);               //dont have to save value and type becauce i will never be searching for it in the fist element
    token_was_war = false;

    do{
        
		/* if (Prec(actual_token) == IDENTIFIER && codegen_expr_active()) { */
		/* 	if (actual_token.type == TOKEN_INT) */
		/* 		codegen_new_expr(INT); */
		/* 	else if (actual_token.type == TOKEN_FLOAT) */
		/* 		codegen_new_expr(FLOAT); */
		/* 	else if (actual_token.type == TOKEN_STRING) */
		/* 		codegen_new_expr(STRING); */
		/* } */
        if(actual_token.type == TOKEN_COLON) {
            is_colon = true;
        }
        
        if (error_num != OK) {

           break;

        }   else if(actual_token.type == TOKEN_FLOAT_EXP) {         // float_exp is stil normal float type
    

            actual_token.type = TOKEN_FLOAT;

        } else if (actual_token.type == TOKEN_INT_EXP) {

           actual_token.type = TOKEN_FLOAT;

        }

         if (where != BOOL_TRUE)   { // bool false is when we are in return function
            
            if (Prec(actual_token) == RELATIONS) {    // cant have an retion operand in assign ....

                //semantic ---------------> expression returning a bool value and not statement
                error_num = TYPE_COMPABILITY_ERROR;
                break;

                
            }
            
        } else {
            if (Prec(s1->symbol) == RELATIONS && is_relation < 2) {          // cant have more than 2 relation in statement and also we have to have one
                is_relation++;
            }  
            
        }

       

        if (actual_token.type == TOKEN_NEW_LINE && s1->symbol.type == TOKEN_DOLLAR) {    // if we have an expression and fist line is empty it is not a fault
                skip_new_line(actual_token);
        }
        
        
// situation when input in expression is empty or it is empty expression which ends with left bracket -> it is syntax error        
        if ((actual_token.type == TOKEN_RIGHT_ROUND ||  Prec(actual_token) == DOLAR) && *&s1->symbol.type == TOKEN_DOLLAR) { 
             
            error_num = SYNTAX_ERROR;
            break;

        }   else if (is_bracket_in_print) {

           break;

        } else if (Prec(actual_token) == NON_SYMBOL_IN_EXPRESSION) {

                if (actual_token.type == TOKEN_UNDEFINED) {     
                    destroy(&s1);
                    error_num = LEXICAL_ERROR; 
                } else {
                   
                    destroy(&s1);
                    error_num = SYNTAX_ERROR;   
                }
                
                break;

        } else if (Prec(actual_token) < RIGHT_BR) {

                is_EOL = true;

        }  else if (is_EOL && actual_token.type == TOKEN_NEW_LINE) { 
                                                                                                                
                skip_new_line(actual_token);
                   
        } else {
            is_EOL = false;
        }
        
        

      

        if(actual_token.type == TOKEN_VAR) {                            //////////// if actual token is var we have to check if this var is declared
        
             if (stab_dict_lookup(dict,actual_token.data.string.string) ==  false) {
                destroy(&(s1));
                return DEFINITION_ERROR;
            }
            token_was_war = true;
	        temp = stab_dict_get(dict, actual_token.data.string.string);               // if actual var is declared we know about it and we can rewrite type from var to actual var type
            actual_token.type = temp->type;
        }
        

        top_terminal = top_terminal_in_stack(&s1);

        
        switch (prec_table[Prec(top_terminal)][Prec(actual_token)])
		{
		case L:

            insert_after_last(&s1);
            push(&s1,&actual_token,symbol_dollar);
            actual_token = getToken();
	    	break;

		case E:

            push(&s1,&actual_token,symbol_dollar);
			actual_token = getToken();            
			break;

		case M:
			error_num = reduce(&s1, dict);
			break;

		case DONE:
			
            ok_done = true;

			break;

        case ERR:

            if (actual_token.type == TOKEN_RIGHT_ROUND) {
                is_bracket_in_print = true;
            } else {
                
                error_num = SYNTAX_ERROR;
            }
            
           
            
			break;
		}
        

    } while (!ok_done);



    if (BOOL_TRUE == where && is_relation != 1 && error_num == OK) {        // cant have more than 2 relation in statement and also we have to have one

                error_num = TYPE_COMPABILITY_ERROR;

    } 
    
     
    if (error_num == OK) {

        if(where != ASSIGN) {
    
            if(where == IN_MULTIPE) {
                
                if (is_colon) {                    
                    switch (s1->type) {
                 
                    case TOKEN_INT :

                        if (return_typ[global_counter_for] != 'i' && return_typ[global_counter_for] != '_') {           //checking if in multiple assign is more elements or if each type is called
							destroy(&s1);
                            return TYPE_COMPABILITY_ERROR;
                        }
                     
                        global_counter_for ++;
						destroy(&s1);
                        return OK;
                    case TOKEN_FLOAT :
                        if (return_typ[global_counter_for] != 'f' && return_typ[global_counter_for] != '_') {
							destroy(&s1);
                            return TYPE_COMPABILITY_ERROR;
                        }
                        global_counter_for ++;
						destroy(&s1);
                        return OK;
                    case TOKEN_STRING :
                        if (return_typ[global_counter_for] != 's' && return_typ[global_counter_for] != '_') {
							destroy(&s1);
                            return TYPE_COMPABILITY_ERROR;
                        }
                        global_counter_for ++;
						destroy(&s1);
                        return OK;      
					default:
						break;
                 }

            } else {
                switch (s1->type) {

                    case TOKEN_INT :
                        if ((return_typ[global_counter_for] != 'i' && return_typ[global_counter_for] != '_') || index_return != (global_counter_for + 1)) {
							destroy(&s1);
                            return SEMANTIC_ERROR;
                        }
                        global_counter_for = 0;
						destroy(&s1);
                        return OK;
                    case TOKEN_FLOAT :
                        if ((return_typ[global_counter_for] != 'f' && return_typ[global_counter_for] != '_') || index_return != (global_counter_for + 1)) {
							destroy(&s1);
                            return SEMANTIC_ERROR;
                        }
                        global_counter_for = 0;
						destroy(&s1);
                        return OK;
                    case TOKEN_STRING :
                        if ((return_typ[global_counter_for] != 's' && return_typ[global_counter_for] != '_') || index_return != (global_counter_for + 1)) {
							destroy(&s1);
                            return SEMANTIC_ERROR;
                        }
                        global_counter_for = 0;
						destroy(&s1);
                        return OK; 
					default:
						break;
                    } 
                }
               
            } else if(where == DECLARATION) {
                       
                global_counter_for = 0;
                    if(where_store_type.type != TOKEN_VAR_UNDERSCORE) {

                        stab_set(scope_table,where_store_type.data.string.string,s1->type);
                    }  else {
             
                        error_num = DEFINITION_ERROR;
                    }

                 destroy(&s1);
                } else  {
                
                    if (where == BOOL_FALSE && is_colon){
                    
                        switch (s1->type) {
                            case TOKEN_INT :
                                if (where_is_key->return_types[global_counter_for] != 'i') {
									destroy(&s1);
                                    return RETURN_ERROR;
                                }
                            global_counter_for ++;
							destroy(&s1);
                            return OK;

                            case TOKEN_FLOAT :
                                if (where_is_key->return_types[global_counter_for] != 'f') {
									destroy(&s1);
                                    return RETURN_ERROR;
                                }
                                global_counter_for ++;
								destroy(&s1);
                                return OK;
                                
                            case TOKEN_STRING :
                                if (where_is_key->return_types[global_counter_for] != 's') {
									destroy(&s1);
                                    return RETURN_ERROR;
                                }
                                global_counter_for ++;
								destroy(&s1);
                                return OK;  
							default:
								break;
                   
                        }
                    } else if (where == BOOL_FALSE && !is_colon){
                
                        switch (s1->type) {
                            case TOKEN_INT :
                                if (where_is_key->return_types[global_counter_for] != 'i' || where_is_key->return_count != (global_counter_for + 1)) {
								destroy(&s1);
                                return PARAM_CNT_ERROR;
                            }
                            global_counter_for = 0;
							destroy(&s1);
                            return OK;
                
                            case TOKEN_FLOAT :
                                if (where_is_key->return_types[global_counter_for] != 'f' || where_is_key->return_count != (global_counter_for + 1)) {
									destroy(&s1);
                                    return PARAM_CNT_ERROR;
                                 }
                            global_counter_for = 0;
							destroy(&s1);
                            return OK;
                            
                            case TOKEN_STRING :
                                if (where_is_key->return_types[global_counter_for] != 's' || where_is_key->return_count != (global_counter_for + 1)) {
								destroy(&s1);
                                return PARAM_CNT_ERROR;
                                }
                            global_counter_for = 0;
							destroy(&s1);
                            return OK;
							default:
								break;
                        } 
                }  else {
                
                    global_counter_for = 0;
                }
                
                destroy(&s1);
            } 
            
        } else {
            
                global_counter_for = 0;
            
                if (where_store_type.type != TOKEN_VAR_UNDERSCORE) {
                
                    temp = stab_dict_get(dict,where_store_type.data.string.string);
                    where_store_type.type = temp->type; 

                    if (s1->type != where_store_type.type ) {
						destroy(&s1);
                        return TYPE_COMPABILITY_ERROR; 
                    }
                }
                
                
        }

    } else {
        destroy(&s1);
    }
  
	destroy(&s1);
    return error_num;
    
}

