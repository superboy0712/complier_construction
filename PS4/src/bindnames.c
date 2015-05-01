#include "bindnames.h"
#include <assert.h>
extern int outputStage; // This variable is located in vslc.c
char* thisClass;

//Solutions to last assignment, precompiled in bindsol.o
int bc(node_t* root, int stackOffset);
int bd(node_t* root, int stackOffset);

int bind_default ( node_t *root, int stackOffset)
{
	return bd(root, stackOffset);
}

int bind_function ( node_t *root, int stackOffset)
{
	/**
	 * function
	 * data_type = {...}
	 * label = "foo"
	 *   	-parameter_list
	 *   	-statement_list
	 */
	if(outputStage == 6)
		printf( "FUNCTION: Start: %s\n", root->label);
	assert(root);
	assert(root->n_children == 2);
	node_t *para_list = root->children[0];
	node_t *stat_list = root->children[1];
	scope_add();
	if(para_list){
		/**
		 * para_list can be NULL after simplification
		 */
		int j = 8 + 4*(para_list->n_children-1);
		for (int i = 0; i < para_list->n_children; ++i) {
			bind_declaration(para_list->children[i], j);
			j -= 4;
		}
	}
	int j = -4;
	for (int i = 0; i < stat_list->n_children; ++i) {
		if(stat_list->children[i]->nodetype.index == declaration_statement_n.index){
			bind_declaration(stat_list->children[i], j);
			j -= 4;
		}else{
			stat_list->children[i]->bind_names(stat_list->children[i], stackOffset);
		}
	}
	scope_remove();
	if(outputStage == 6)
		printf( "FUNCTION: End\n");
	return 0;
}




function_symbol_t* create_function_symbol(node_t* function_node)
{
	/**
	 * function
	 * data_type = {...}
	 * label = "foo"
	 *   	-parameter_list
	 *   	-statement_list
	 */
	assert(function_node);
	function_symbol_t *new_func_symbol = calloc(1, sizeof(function_symbol_t));
	if(new_func_symbol == NULL){
		perror("calloc");
		exit(EXIT_FAILURE);
	}
	node_t *para_list = function_node->children[0];
	if(para_list){
		new_func_symbol->nArguments = para_list->n_children;
		new_func_symbol->argument_types = calloc(para_list->n_children, sizeof(data_type_t));
		if(new_func_symbol->argument_types == NULL){
			perror("calloc");
			exit(EXIT_FAILURE);
		}
		for (int i = 0; i < para_list->n_children; ++i) {
			new_func_symbol->argument_types[i]
				= para_list->children[i]->data_type;
		}
	}
	new_func_symbol->label = STRDUP(function_node->label);
	new_func_symbol->return_type = function_node->data_type;

	return new_func_symbol;
}

int bind_function_list ( node_t *root, int stackOffset)
{
	if(outputStage == 6)
		printf( "FUNCTION_LIST: Start\n");
	scope_add();
	/**
	 * create new function symbol table
	 */
	for(int i=0; i<root->n_children; i++){
		function_symbol_t *new = create_function_symbol(root->children[i]);
		function_add((root->children[i]->label), new);
		//bind_function(root->children[i], stackOffset);
	}
	bind_default(root, stackOffset);
	scope_remove();
	if(outputStage == 6)
		printf( "FUNCTION_LIST: End\n");
	return 0;
}

int bind_constant ( node_t *root, int stackOffset)
{
    return bc(root, stackOffset);
}


symbol_t* create_symbol(node_t* declaration_node, int stackOffset)
{
    /**
     *  declaration
     *  data_type = {...}
     *  label = "foo"
     */
	assert(declaration_node);
	symbol_t *new_symbol = malloc(sizeof(symbol_t));
	if(!new_symbol){
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	new_symbol->depth = 0;
	new_symbol->label = STRDUP(declaration_node->label);
	new_symbol->stack_offset = stackOffset;
	new_symbol->type = declaration_node->data_type;
	return new_symbol;
}

int bind_declaration ( node_t *root, int stackOffset)
{
	if(outputStage == 6)
		printf( "DECLARATION: parameter/variable : '%s', offset: %d\n", root->label, stackOffset);
	//bind_default(root, stackOffset);
	symbol_t *new_symbol = create_symbol(root, stackOffset);
	symbol_insert(new_symbol->label, new_symbol);
	return 0;
}

int bind_variable ( node_t *root, int stackOffset)
{
	/**
	 * make sure it's masked in declaration part (symbol_insert)
	 * and also masked out int call_e
	 * here we retrieve (symbol_get)
	 */
	if(outputStage == 6)
		printf( "VARIABLE: access: %s\n", root->label);
	root->entry = symbol_get(root->label);
	return 0;

}

int bind_expression( node_t* root, int stackOffset)
{
	if(outputStage == 6)
		printf( "EXPRESSION: Start: %s\n", root->expression_type.text);
	if(root->expression_type.index == FUNC_CALL_E){
		/**
		 * variable expression_list
		 */
		root->function_entry = function_get(root->children[0]->label);
		if(root->children[1]){
			bind_default(root->children[1], stackOffset);
		}
	}else{
	   bind_default(root, stackOffset);
	}

	if(outputStage == 6)
		printf( "EXPRESSION: End\n");
	return 0;
}


