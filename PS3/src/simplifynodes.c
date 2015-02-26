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
	if(root->n_children == 1){
		return root;
	}
	assert(root->n_children == 2);
	Node_t * right = root->children[root->n_children -1 ];
	Node_t * left = root->children[0];
	assert(left != right);
	// assume the leftmost child also is a sorted/simplified list type node
	if(left->children != NULL){
		// spare more space of the children array
		if(
				realloc(
						root->children,
						(left->n_children + 1) * sizeof(Node_t *)
						) == NULL
		){
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
		left->children = NULL;
		free(left);
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
	return root;
}

