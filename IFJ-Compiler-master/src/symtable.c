/**
 * symtable.c - implementation of abstact data structure to store symbols
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

#include "symtable.h"
// these macros should be local, that's why they are not in .h file
#define ABS(x) ((x) < 0 ? -(x) : (x))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define INIT_LEAF 1

static int num_of_digits(int num)
{
	int cnt = 0;
	do {
		cnt++;
		num /= 10;
	} while (num != 0);
	return cnt;
}

bool make_ir(node_t *new_node)
{
	static int token_cnt = 1;
	new_node->ir = (char *)malloc(sizeof(char) *
						(TSTR_SIZE + num_of_digits(token_cnt)));
	if (new_node->ir == NULL)
		return false;
	// TODO sprintf is faulty, I need replacement, maybe two buffers?
	sprintf(new_node->ir, "LF@T%d", token_cnt);
	token_cnt++;
	return true;
}

/**
 * Private function that encodes key to numerical value for AVL purposes
 */
static int encode(const char *key)
{
	int encoded = 0;
	for (size_t i = 0; i < strlen(key); i++)
		encoded += (int)key[i];
	return encoded;
}

/**
 * Initializes root to default values and pushes it to dictionary
 */
root_t stab_root_init(stab_dict_t dict)
{
	if (dict == NULL)
		return NULL;
	root_t root = (root_t)malloc(sizeof(struct root_node));
	if (root == NULL)
		return NULL;
	root->node = NULL;
	stab_dict_push(dict, root);
	return root;
}

/**
 * Private function that returns height of a subtree ~ tree
 */
static inline int tree_height(node_t *root)
{
	return (root == NULL ? 0 : root->height);
}

/**
 * Private function that decides whether the tree is balanced
 */
static int tree_balance(node_t *root)
{
	if (root == NULL)
		return 0;
	return (tree_height(root->left) - tree_height(root->right));
}

/**
 * Private function that updates height of said node
 */
static inline int update_height(node_t *root)
{
	return (1 + MAX(tree_height(root->left), tree_height(root->right)));
}

/**
 * Private function that rotates subtree to the left
 */
static node_t *rotate_right(node_t *root)
{
	node_t *new_root = root->left;
	node_t *temp = new_root->right;

	// Rotation
	new_root->right = root;
	root->left = temp;

	root->height = update_height(root);
	new_root->height = update_height(new_root);
	return new_root;
}

/**
 * Private function that rotates subtree to the left
 */
static node_t *rotate_left(node_t *root)
{
	node_t *new_root = root->right;
	node_t *temp = new_root->left;

	// Rotation
	new_root->left = root;
	root->right = temp;

	root->height = update_height(root);
	new_root->height = update_height(new_root);
	return new_root;
}

/**
 * Private function that balances newly inserted nodes
 */
static node_t *balanced_node(int balance, node_t *root, int id)
{
	if (balance > 1 && id < root->left->id)
		return rotate_right(root);
	if (balance < -1 && id > root->right->id)
		return rotate_left(root);
	if (balance > 1 && id > root->left->id) {
		root->left = rotate_left(root->left);
		return rotate_right(root);
	}
	if (balance < -1 && id < root->right->id) {
		root->right = rotate_right(root->right);
		return rotate_left(root);
	}
	return root;
}

/**
 * Private function that appends node to the end of a list
 */
static void stab_append_list(node_t *root, node_t *new_item)
{
	while (root->next != NULL)
		root = root->next;
	root->next = new_item;
}

/**
 * Private function that creates new node
 */
static node_t *stab_new(int id, char *key)
{
	node_t *new_node = (node_t *)malloc(sizeof(node_t));

	new_node->id = id;
	new_node->key = (char *)malloc(sizeof(char) * strlen(key) + 1);
	new_node->key = strcpy(new_node->key, key);
	new_node->next = NULL;
	new_node->left = NULL;
	new_node->right = NULL;
	new_node->height = INIT_LEAF;

	make_ir(new_node);
	return new_node;
}

/**
 * Private function that inserts new node into the list and rebalances the
 * tree as needed
 */
static node_t *stab_new_insert(node_t *root, int id, char *key)
{
	if (root == NULL)
		return stab_new(id, key);

	if (id < root->id)
		root->left = stab_new_insert(root->left, id, key);
	else if (id > root->id)
		root->right = stab_new_insert(root->right, id, key);
	// in case there would be key encoding conflict
	else if (id == root->id)
		stab_append_list(root, stab_new(id, key));
	else
		return root;

	root->height = update_height(root);
	int balance = tree_balance(root);
	return balanced_node(balance, root, id);
}

/**
 *
 */
void stab_insert(root_t root, char *key)
{
	if (root == NULL)
		return;
	root->node = stab_new_insert(root->node, encode(key), key);
}

/**
 * Private function that frees list or single node that in reality is a list
 * of single element
 */
static void free_list(node_t *root)
{
	node_t *tmp;
	while (root != NULL) {
		tmp = root;
		root = root->next;
		free(tmp->key);
		free(tmp);
	}
}

/**
 * Private function, used recursively free a whole subtree
 */
static void free_tree(node_t *root)
{
	if (root == NULL)
		return;
	if (root->left != NULL)
		free_tree(root->left);
	if (root->right != NULL)
		free_tree(root->right);
	free_list(root);
}

/**
 * Public destroy function, expect root as input, frees the whole subtree
 */
void stab_destroy(root_t root)
{
	free_tree(root->node);
	free(root);
}

/**
 * Private function that finds node in tree
 */
static node_t *stab_find(node_t *root, int id)
{
	while(root != NULL) {
		if (id > root->id)
			root = root->right;
		else if (id < root->id)
			root = root->left;
		else
			return root;
	}
	return NULL;
}

/**
 * Private function that matches key with node in list
 */
static node_t *stab_match(node_t *root, const char *key)
{
	while(root != NULL) {
		if (!strcmp(root->key, key))
			return root;
		root = root->next;
	}
	return NULL;
}

/**
 * returns node that has the same id or NULL, if there is no such node
 */
node_t *stab_get(root_t root, const char *key)
{
	return stab_match(stab_find(root->node, encode(key)), key);
}

/**
 * sets value to node specified by key 
 */
bool stab_set(root_t root, const char *key, token_t token_type)
{
	node_t *mutated_node;
	if ((mutated_node = stab_get(root, key)) == NULL)
		return false;
	mutated_node->type = token_type;
	return true;
}


/**
 * stab_function_set(list->global->root,token.data.string,tvoje_tmp_node);
 */
bool stab_function_set(root_t root, char *key, node_t* tmp_node){
	
	stab_insert(root,key);
	node_t *mutated_node;

	if ((mutated_node = stab_get(root, key)) == NULL)
		return false;

	mutated_node->param_count = tmp_node->param_count;
	mutated_node->declared = tmp_node->declared;
	mutated_node->return_count = tmp_node->return_count;
	strcpy(mutated_node->params,tmp_node->params);
	strcpy(mutated_node->return_types,tmp_node->return_types);

	return true;
}
static bool stab_functions_nexts(node_t *root)
{
	while(root != NULL) {
		if (!(root->declared))
			return false;
		root = root->next;
	}
	return true;
}
bool stab_are_functions_defined(node_t* root){
	
	if(root == NULL)
		return true;
	
	if(!stab_functions_nexts(root))
		return false;

	bool b1 = stab_are_functions_defined(root->left);
	bool b2 = stab_are_functions_defined(root->right);
	
	if(b1 && b2)
		return true;
	else
		return false;
	
}

/**
 * Looks up keyword in symtable
 */
bool stab_lookup(root_t root, const char *key)
{
	if (stab_match(stab_find(root->node, encode(key)), key) == NULL)
		return false;
	return true;
}

/**
 * Looks up keyword in all symtabels across the dictionary
 */
bool stab_dict_lookup(stab_dict_t head, const char *key)
{
	stab_t *tmp = head->head;
	while (tmp != NULL) {
		if (stab_lookup(tmp->root, key))
			return true;
		tmp = tmp->next;
	}	
	return false;
}

/**
 * Gets pointer to the node that is represented by keyword
 */
node_t *stab_dict_get(stab_dict_t head, const char *key)
{
	stab_t *tmp = head->head;
	node_t *ret_val = NULL;
	while (tmp != NULL) {
		ret_val = stab_get(tmp->root, key);
		if (ret_val != NULL)
			return ret_val;
		tmp = tmp ->next;
	}
	return NULL;
}

/**
 * Initializes dictionary to default values
 */
stab_dict_t stab_dict_init()
{
	stab_dict_t dict = (stab_dict_t)malloc(sizeof(struct stab_dict));
	if (dict == NULL)
		return NULL;
	dict->head = NULL;
	dict->global = NULL;
	return dict;
}

/**
 * Pushes symtable to the dictionary
 */
bool stab_dict_push(stab_dict_t dict, root_t root)
{
	stab_t *tmp = dict->head;
	dict->head = (stab_t *)malloc(sizeof(struct stab));
	if (dict->head == NULL) {
		dict->head = tmp;
		return false;
	}
	
	
	dict->head->root = root;
	dict->head->next = tmp;
	
	if(tmp == NULL)
		dict->global=dict->head;
	return true;
}

/* NOTE: Freeing is still not working in 100% of times */
/**
 * Pops symtable from the dictionary
 */
bool stab_dict_pop(stab_dict_t dict)
{
	if (dict->head == NULL) // list is empty
		return false;

	stab_t *tmp = dict->head;
	dict->head = dict->head->next;

	if(dict->head == NULL){
		dict->global = NULL;
		dict->head = NULL;
	}
	free(tmp);
	return true;
}

/**
 * Disposes the whole dictionary
 */
void stab_dict_dispose(stab_dict_t dict)
{
	if (dict == NULL)
		return;
	while (stab_dict_pop(dict))
		;
	free(dict);
}
