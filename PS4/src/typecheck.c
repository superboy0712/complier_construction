#include "typecheck.h"
#include <assert.h>
extern int outputStage;

static int type_errors = 0;

int has_type_errors() {
	return type_errors;
}
data_type_t null_data_type = { 0 };
void type_error(node_t* root) {
	fprintf(stdout, "Type error at:\n");
	if (root != NULL) {
		fprintf(stdout, "%s", root->nodetype.text);
		if (root->nodetype.index == EXPRESSION) {
			fprintf(stdout, " (%s)", root->expression_type.text);
		}
		fprintf(stdout, "\n");
	}
	type_errors++;
}

int equal_types(data_type_t a, data_type_t b) {

	if (a.base_type == INT_TYPE || a.base_type == FLOAT_TYPE
			|| a.base_type == BOOL_TYPE || a.base_type == STRING_TYPE) {
		return a.base_type == b.base_type;
	}
	else if (a.base_type == ARRAY_TYPE) {
		int equals = b.base_type == ARRAY_TYPE;
		equals = equals && (a.array_type == b.array_type);
		equals = equals && (a.n_dimensions == b.n_dimensions);
		if (!equals) {
			return equals;
		}
		for (int i = 0; i < a.n_dimensions; i++) {
			equals = equals && (a.dimensions[i] == b.dimensions[i]);
		}

		return equals;
	}
	else {
		return 0;
	}
}

data_type_t typecheck_default(node_t* root) {
	if (!root) {
		return null_data_type;
	}
	for (int i = 0; i < root->n_children; ++i) {
		if (root->children[i]) {
			root->children[i]->typecheck(root->children[i]);
		}
	}
	return root->data_type;
}

data_type_t typecheck_sub_tree(node_t* root) {
	if (!root) {
		return null_data_type;
	}
	root->typecheck(root);
	return root->data_type;
}

data_type_t typecheck_expression(node_t* root) {

	//You may need to add code at various places to complete this function

	if (outputStage == 10)
		printf("Type checking expression %s\n", root->expression_type.text);
	if (root->n_children == 0) {
		return root->data_type;
	}
	else if (root->n_children == 1) {

		switch (root->expression_type.index) {
		case UMINUS_E:
		{
			data_type_t expr = typecheck_default(root->children[0]);
			if(expr.base_type != INT_TYPE && expr.base_type != FLOAT_TYPE){
				type_error(root->children[0]);
				type_error(root);
			}else{
				root->data_type.base_type = expr.base_type;
			}
		}
			break;

		case NOT_E:
		{
			data_type_t expr = typecheck_default(root->children[0]);
			if(expr.base_type != BOOL_TYPE){
				type_error(root->children[0]);
				type_error(root);
			}else{
				root->data_type.base_type = BOOL_TYPE;
			}
		}
			break;

		default:
			root->data_type = root->children[0]->data_type;
			//type_error(root);
			break;
		}
	}
	else if (root->n_children > 1) {
		typecheck_default(root);
		switch (root->expression_type.index) {
		case ADD_E:
		case SUB_E:
		case DIV_E:
		case MUL_E:
		{
			if((root->children[0]->data_type.base_type != INT_TYPE
				&&root->children[0]->data_type.base_type != FLOAT_TYPE)
				||
				(root->children[1]->data_type.base_type != INT_TYPE
				&&root->children[1]->data_type.base_type != FLOAT_TYPE)){

				type_error(root);
			}else if(!equal_types(
					root->children[0]->data_type  ,
					root->children[1]->data_type
							)
			){
				type_error(root);
			}else{
				root->data_type = root->children[0]->data_type;
			}
		}
			break;

		case LEQUAL_E:
		case GEQUAL_E:
		case GREATER_E:
		case LESS_E:
		{
			if(
				(root->children[0]->data_type.base_type != INT_TYPE
				&&root->children[0]->data_type.base_type != FLOAT_TYPE)
				||
				(root->children[1]->data_type.base_type != INT_TYPE
				&&root->children[1]->data_type.base_type != FLOAT_TYPE)){

				type_error(root);
			}else if(!equal_types(
					root->children[0]->data_type  ,
					root->children[1]->data_type
							)
			){
				type_error(root);
			}else{
				root->data_type.base_type = BOOL_TYPE;
			}
		}
			break;

		case AND_E:
		case OR_E:
		{
			if(root->children[0]->data_type.base_type != BOOL_TYPE
				||root->children[1]->data_type.base_type != BOOL_TYPE){

				type_error(root);
			}else if(!equal_types(
					root->children[0]->data_type  ,
					root->children[1]->data_type
							)
			){
				type_error(root);
			}else{
				root->data_type.base_type = BOOL_TYPE;
			}
		}
			break;

		case EQUAL_E:
		case NEQUAL_E:
		{
			if(
				(root->children[0]->data_type.base_type != INT_TYPE
				&&root->children[0]->data_type.base_type != FLOAT_TYPE
				&&root->children[0]->data_type.base_type != BOOL_TYPE)
				||
				(root->children[1]->data_type.base_type != INT_TYPE
				&&root->children[1]->data_type.base_type != FLOAT_TYPE
				&&root->children[1]->data_type.base_type != BOOL_TYPE)){

				type_error(root);
			}else if(!equal_types(
					root->children[0]->data_type  ,
					root->children[1]->data_type
							)
			){
				type_error(root);
			}else{
				root->data_type.base_type = BOOL_TYPE;
			}
		}
			break;

		case FUNC_CALL_E:
		{
			/**
			 * function_e access
			 * call_e := variable expression_list
			 */
			/** check if calling expression compiant to definition */
			int correct = 1;
			if(root->children[1]){
				if(root->function_entry->nArguments != root->children[1]->n_children){
					// nArguments
					correct = 0;
					type_error(root);
				}else if(correct){
					// arguments types
					for (int i = 0; i < root->children[1]->n_children; ++i) {
						correct = !memcmp(&root->function_entry->argument_types[i],
								&root->children[1]->children[i]->data_type,
								sizeof(data_type_t));

						if(!correct){
							type_error(root);
							break;
						}
					}
				}
			}
			if(correct){
				// return type
				correct = !memcmp(&root->function_entry->return_type
							, &root->data_type
							,sizeof(data_type_t));
				if(!correct){
						type_error(root);
				}
			}
		}
			break;

		case ARRAY_INDEX_E:
			break;

		default:
			type_error(root);
			break;
		}
	}
	return root->data_type;
}

data_type_t typecheck_variable(node_t* root) {
	//variable ::= IDENTIFIER
	assert(root->nodetype.index == variable_n.index);
	return root->data_type;
}

data_type_t typecheck_assignment(node_t* root) {
	if (outputStage == 10) {
		printf("Type checking assignment\n");
	}
	/**
	 * assignment statement ::= lvalue ASSIGN expression
	 */
	assert(root);
	assert(root->n_children == 2);
//	data_type_t lval = typecheck_sub_tree(root->children[0]);
//	data_type_t rval = typecheck_sub_tree(root->children[1]);
	typecheck_default(root);
	if (!equal_types(root->children[0]->data_type,
			root->children[1]->data_type)) {
		type_error(root);
	}
	return root->data_type;
}
data_type_t typecheck_function_definition(node_t* root){
	assert(root->children[1]);
	node_t *stat_list = root->children[1];
	typecheck_default(root);
	for (int i = 0; i < stat_list->n_children; ++i) {
		if(root->children[i]){
			if(root->nodetype.index == return_statement_n.index){
				/**
				 * return statement ::= RETURN expression
				 *
				 * checking if the return expression matches the definied function "head"
				 */
				//typecheck_default(root->children[i]);
				assert(root->children[i]->n_children == 1);
				assert(root->children[i]->children[0]);
				if(memcmp(&root->data_type,
						&root->children[i]->children[0]->data_type,
						sizeof(data_type_t))){
					type_error(root);
					break;
				}
			}else{
				//typecheck_sub_tree(root->children[i]);
			}
		}
	}
	return root->data_type;
}
