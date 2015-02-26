#include "simplifynodes.h"
#include <assert.h>
extern int outputStage; // This variable is located in vslc.c

Node_t* simplify_default ( Node_t *root, int depth )
{
	if(root == NULL){
		return NULL;
	}
	for( int i = 0; i < root->n_children; i++){
		if(root->children[i] != NULL){
			root->children[i]->simplify(root->children[i], depth + 1);
		}
	}
	return root;
}


Node_t *simplify_types ( Node_t *root, int depth )
{
	if(root == NULL){
		return NULL;
	}
	for( int i = 0; i < root->n_children; i++){
			if(root->children[i] != NULL){
				root->children[i]->simplify(root->children[i], depth + 1);
			}
	}
	if(root->data_type.base_type == ARRAY_TYPE){
		root->data_type.array_type = root->children[0]->data_type.base_type;
		node_finalize(root->children[0]);
		root->children[0] = NULL;
		root->data_type.n_dimensions = root->children[1]->n_children;
		int n = root->children[1]->n_children;
		int* index = malloc(n*sizeof(int));
		for (int i = 0; i < n; ++i) {
			index[i] = root->children[1]->children[i]->int_const;
			node_finalize(root->children[1]->children[i]);
			root->children[1]->children[i] = NULL;
		}
		root->data_type.dimensions = index;
		node_finalize(root->children[1]);
		root->children[1] = NULL;
		free(root->children);
		root->children = NULL;
		root->n_children = 0;
	}
	return root;
}


Node_t *simplify_function ( Node_t *root, int depth )
{
	if(root == NULL){
		return NULL;
	}
	for( int i = 0; i < root->n_children; i++){
		if(root->children[i] != NULL){
			root->children[i]->simplify(root->children[i], depth + 1);
		}
	}
	if(root->nodetype.index != function_n.index)
		return root;
	assert(root->n_children == 4);
	root->data_type = root->children[0]->data_type;
		root->label = STRDUP((root->children[1]->label));
		if(root->label == NULL){
			perror("STRDUP in simplify function");
			exit(EXIT_FAILURE);
		}
	Node_t * pl = root->children[2];
	Node_t * sl = root->children[3];

	//node_finalize()
	node_finalize(root->children[0]);
		root->children[0] = NULL;
	//free(root->children[1]->label);/* need to free */
	node_finalize(root->children[1]);
		root->children[1] = NULL;
		root->children[2] = NULL;
		root->children[3] = NULL;
	free(root->children);
	root->children = malloc(2*sizeof(Node_t *));
	if(root->children == NULL){
		perror("malloc in simplify function");
		exit(EXIT_FAILURE);
	}
	root->children[0] = pl;
	root->children[1] = sl;

	root->n_children = 2; /* very important */

	pl = NULL;
	sl = NULL;

	return root;
}




Node_t *simplify_declaration_statement ( Node_t *root, int depth )
{
	if(root == NULL){
		return NULL;
	}
	for( int i = 0; i < root->n_children; i++){
		if(root->children[i] != NULL){
			root->children[i]->simplify(root->children[i], depth + 1);
		}
	}
	if(root->nodetype.index != declaration_statement_n.index)
		return root;
	assert(root->n_children == 2);
	root->data_type = root->children[0]->data_type;
	node_finalize(root->children[0]);
	root->children[0] = NULL;
	root->label = STRDUP(root->children[1]->label);
	node_finalize(root->children[1]);
	root->children[1] = NULL;
	free(root->children);
	root->children = NULL;
	root->n_children = 0;
	return root;
}


Node_t *simplify_single_child ( Node_t *root, int depth )
{
	if(root == NULL){
		return NULL;
	}
	for( int i = 0; i < root->n_children; i++){
		if(root->children[i] != NULL){
			root->children[i]->simplify(root->children[i], depth + 1);
		}
	}
	/* argument_list, statements, parameter_lists,
	 * expression
	 * except new_e, uminus_e and not_e */
	if(root->n_children != 1)
		return root;

	/* copy the whole content of the child to me */
	Node_t * child = root->children[0];
	if(child != NULL){
		*root = *child;
		root->children = malloc(child->n_children * sizeof(Node_t *));
		if(root->children == NULL){
			perror("malloc in simplify single child");
			return root;
		}
		memcpy(root->children, child->children, child->n_children * sizeof(Node_t *));
		if(child->label!= NULL){
			root->label = STRDUP(child->label);
		}
		node_finalize(child);
		child = NULL;
	}
	return root;
}

Node_t *simplify_list_with_null ( Node_t *root, int depth )
{
	if(root == NULL){
		return NULL;
	}
	if(outputStage == 4)
		printf( "%*cSimplify %s \n", depth, ' ', root->nodetype.text );

	for( int i = 0; i < root->n_children; i++){
		if(root->children[i] != NULL){
			//simplify_list_with_null(root->children[i], depth + 1);
			// brilliant here, don't recurse in the wrong way
			root->children[i]->simplify(root->children[i], depth + 1);
		}else if(root->children[i + 1] != NULL){
			// move the successor elements one step ahead, reallocate the size
			if(memmove(&(root->children[i]), &(root->children[i + 1]), (root->n_children - i -1)*sizeof(Node_t *) ) == NULL){
				perror("memmove");
				exit(EXIT_FAILURE);
			}
			// decrease n_childeren
			root->n_children--;/* this cause next simplification never called */
			i--; /* very important!! */
			// reallocate the size
			root->children = realloc(root->children, (root->n_children)*sizeof(Node_t *));
			if(root->children == NULL){
				perror("realloc");
				exit(EXIT_FAILURE);
			}
		}
	}

	return root;
}


Node_t *simplify_list ( Node_t *root, int depth )
{
	if(root == NULL){
			perror("simplify_list, null node");
			return NULL;
		}
	if(outputStage == 4)
		printf( "%*cSimplify %s \n", depth, ' ', root->nodetype.text );
	
	for(int i = 0; i < root->n_children; i++){
		if(root->children[i] != NULL){
			root->children[i]->simplify(root->children[i], depth + 1);
		}
//      some lists need to reserve the null child
//		else {
//			simplify_list_with_null (root, depth);
//		}
	}
	/* only one node, no need to flatten */
	if(root->n_children < 2){
		return root;
	}
	assert(root->n_children == 2);
	Node_t * right = root->children[1];
	Node_t * left = root->children[0];
	assert(left != right);

	// assume the leftmost child also is a sorted/simplified list type node
	if(left->children != NULL){
		// spare more space of the children array
		root->children = realloc(
								root->children,
								(left->n_children + 1) * sizeof(Node_t *)
								);
		if(root->children == NULL){
				perror("realloc in list simplify");
				exit(EXIT_FAILURE);
		}
		// increase n_children
		root->n_children = left->n_children + 1;
		// copy the grand children to new children array
		for (int i = 0; i < left->n_children; ++i) {
			root->children[i] = left->children[i];
		}
		// copy the rightmost child to new array
		root->children[left->n_children] = right;
		// free the old child
		free(left->children);
		left->children = NULL;
		node_finalize(left);
		left = NULL;
	}

	return root;
}


Node_t *simplify_expression ( Node_t *root, int depth )
{
	if(root == NULL){
		return NULL;
	}
	for( int i = 0; i < root->n_children; i++){
		if(root->children[i] != NULL){
			root->children[i]->simplify(root->children[i], depth + 1);
		}
	}

	if(root->n_children != 1){
		if(root->n_children != 2)
			return root;
		/* evaluating my value */

		return root;
	}
	//simplify_single_child(root, depth);
	/* copy the whole content of the child to me */
	/* except new_e, uminus_e and not_e */
	if((root->expression_type.index == new_e.index)
			||(root->expression_type.index == uminus_e.index)
			||(root->expression_type.index == not_e.index)
			){
		return root;
	}
	Node_t * child = root->children[0];
	if(child != NULL){
		*root = *child;
		root->children = malloc(child->n_children * sizeof(Node_t *));
		if(root->children == NULL){
			perror("malloc in simplify expression single child");
			return root;
		}
		memcpy(root->children, child->children, child->n_children * sizeof(Node_t *));
		if(child->label!= NULL){
			root->label = STRDUP(child->label);
		}
		node_finalize(child);
		child = NULL;
	}

	return root;
}

