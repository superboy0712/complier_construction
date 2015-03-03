#include "bindnames.h"
extern int outputStage; // This variable is located in vslc.c
char* thisClass;

int bind_default ( node_t *root, int stackOffset)
{
	if(root == NULL)
		return 0;
	for (int i = 0; i < root->n_children; ++i) {
		if(root->children[i] != NULL)
		{
			root->children[i]->bind_names(root->children[i], stackOffset +1);
		}
	}
	return 0;
}



int bind_constant ( node_t *root, int stackOffset)
{
	if(root == NULL)
		return 0;
	for (int i = 0; i < root->n_children; ++i) {
		if(root->children[i] != NULL)
		{
			root->children[i]->bind_names(root->children[i], stackOffset +1);
		}
	}

	if(outputStage == 6)
		printf( "CONSTANT\n");

	return 0;
}





