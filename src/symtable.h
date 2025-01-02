/**
 * symtable.h - header for symtable functions
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
 *   xkrizd03 - Daniel Kříž
 */

#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "scanner.h"
// #include "codegen.h"

#define TSTR_SIZE 5

/**
 * @struct node_t symtable.h "symtable.h"
 * @brief represends one entry to the symtable
 */
typedef struct node {
	int id;
	char *key;
	token_t type;
	char *ir;
	int param_count;	
	char params[20];    /*fe: "ii" - integer, integer */
	bool declared;
	int return_count;
	char return_types[20];
	// AVL tree related data, "private" part of structure
	int height;
	struct node *next;
	struct node *left;
	struct node *right;
} node_t;

/**
 * @struct root_t symtable.h "symtable.h"
 * @brief represends root of the symbolic tree
 */
struct root_node {
	node_t *node;
};
typedef struct root_node * root_t;

/**
 * @struct stab_t symtable.h "symtable.h"
 * @brief represends one symbolic table in dictionary
 */
typedef struct stab {
    root_t root;
    struct stab *next;
} stab_t;

/**
 * @struct stab_dict_t symtable.h "symtable.h"
 * @brief represends pointer to the global symtable dictionary
 */
struct stab_dict {
	stab_t *head;
	stab_t *global;
};
typedef struct stab_dict * stab_dict_t;

/**
 * @brief Initializes new root
 * @return returns new root, if allocation was correct, otherwise NULL
 */
root_t stab_root_init();

/**
 * @brief Inserts new node to the tree
 * @post adds node to the tree, everything set to be empty, but key and id
 * @param root root node of a symbolic table tree
 * @param key string that contains the name of symbol
 * @return currently inserted node
 */
void stab_insert(root_t root, char *key); //stab_insert(list->global->root,)

/**
 * @brief Looksup it there is entry with the same key
 * @param root, represends root of symtable
 * @return true if there is entry with that key, otherwise false
 */
bool stab_lookup(root_t root, const char *key);

/**
 * @brief Destroy (frees from memory) whole tree/subtree
 * @pre It destroy tree recursively, don't use it on single node
 * @param root, pointer to the root of symtable
 */
void stab_destroy(root_t root);

/**
 * @brief sets value to the entry specified by key
 * @param root, pointer to the root of symtable
 * @param key, string that represends the symbol
 * @param value, value to be set in entry
 * @return returns true, if value was set (the entry exists), otherwise false
 */
bool stab_set(root_t root, const char *key, token_t token_type);
/**
 * @brief sets value to the entry specified by key
 * @param root, pointer to the root of symtable
 * @param key, string that represends the symbol
 * @param tmp_node, node with parameters, which are copied to new key
 * @return returns true, if value was set (the entry exists), otherwise false
 */
bool stab_function_set(root_t root, char *key, node_t* tmp_node);
/**
 * @brief Gives caller a pointer to entry specified by key
 * @param root, pointer to the root of symtable
 * @param key, string that represends the symbol
 * @return pointer to the entry, or NULL if entry doesn't exist
 */
node_t *stab_get(root_t root, const char *key);

/**
 * @brief Initializes dictionary to the default values
 * @return returns pointer to newly made dictionary, otherwise false
 */
stab_dict_t stab_dict_init();

/**
 * @brief Determines if key is located somewhere in the dictionary
 * @param dict, dictionary to be searched
 * @param key, string that represends the symbol
 * @return true, if the key is in any symtable in dictionary, otherwise false
 */
bool stab_dict_lookup(stab_dict_t dict, const char *key);

/**
 * @brief Gets pointer to node that is specified by key
 * @param dict, dictionary to be searched
 * @param key, string that represends the symbol
 * @return node that is represented by key, otherwise NULL
 */
node_t *stab_dict_get(stab_dict_t dict, const char *key);

/**
 * @brief Pushes symtable to the dictionary of tables
 * @param dict, dictionary to which it is going to be pushed
 * @param root, value to be pushed
 * @return true, if stab was pushed otherwise false
 */
bool stab_dict_push(stab_dict_t dict, root_t root);

/**
 * @brief Pops the first member (head) from dictionary
 * @param dict, dictionary from which it is going to be popped
 * @return true, if stab tab was popped otherwise false
 */
bool stab_dict_pop(stab_dict_t dict);

/**
 * @brief Disposes (frees from memory) the list of symtables
 * @param dict is the dictionary to be disposed
 */
void stab_dict_dispose(stab_dict_t dict);


/**
 * @brief Determines if key is located somewhere in the dictionary
 * @param root, Root to sym table, where you want to know if all functions are defined
 * @return True if all functions are defined, false if not
 */
bool stab_are_functions_defined(node_t *root);

#endif // SYMTABLE_H
