#include "bindnames.h"
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
	if(outputStage == 6)
		printf( "FUNCTION: Start: %s\n", root->label);

        

	if(outputStage == 6)
		printf( "FUNCTION: End\n");
}




function_symbol_t* create_function_symbol(node_t* function_node)
{
    
}

int bind_function_list ( node_t *root, int stackOffset)
{
	if(outputStage == 6)
		printf( "FUNCTION_LIST: Start\n");


	if(outputStage == 6)
		printf( "FUNCTION_LIST: End\n");
}

int bind_constant ( node_t *root, int stackOffset)
{
    return bc(root, stackOffset);
}


symbol_t* create_symbol(node_t* declaration_node, int stackOffset)
{
    
}

int bind_declaration ( node_t *root, int stackOffset)
{
	if(outputStage == 6)
		printf( "DECLARATION: parameter/variable : '%s', offset: %d\n", root->label, stackOffset);


}

int bind_variable ( node_t *root, int stackOffset)
{
	if(outputStage == 6)
		printf( "VARIABLE: access: %s\n", root->label);


}

int bind_expression( node_t* root, int stackOffset)
{
	if(outputStage == 6)
		printf( "EXPRESSION: Start: %s\n", root->expression_type.text);


	if(outputStage == 6)
		printf( "EXPRESSION: End\n");
}


