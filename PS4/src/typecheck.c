#include "typecheck.h"
extern int outputStage;

static int type_errors = 0;

int has_type_errors(){
	return type_errors;
}

void type_error(node_t* root){
        fprintf(stdout, "Type error at:\n");
        if(root != NULL){
                fprintf(stdout,"%s", root->nodetype.text);
                if(root->nodetype.index == EXPRESSION){
                        fprintf(stdout," (%s)", root->expression_type.text);
                }
                fprintf(stdout,"\n");
        }
        type_errors++;
}

int equal_types(data_type_t a, data_type_t b){

	if(a.base_type == INT_TYPE || a.base_type == FLOAT_TYPE || a.base_type == BOOL_TYPE || a.base_type == STRING_TYPE){
		return a.base_type == b.base_type;
	}
	else if(a.base_type == ARRAY_TYPE){
		int equals = b.base_type == ARRAY_TYPE;
		equals = equals && (a.array_type == b.array_type);
		equals = equals && (a.n_dimensions == b.n_dimensions);
		if(!equals){
			return equals;
		}
		for(int i = 0; i < a.n_dimensions; i++){
			equals = equals && (a.dimensions[i] == b.dimensions[i]);
		}

		return equals;
	}
	else{
		return 0;
	}
}

data_type_t typecheck_default(node_t* root)
{
    
}

data_type_t typecheck_expression(node_t* root)
{

    //You may need to add code at various places to complete this function
    
	if(outputStage == 10)
		printf( "Type checking expression %s\n", root->expression_type.text);

	if(root->n_children == 0){
            
	}
	else if(root->n_children == 1){
            
            switch(root->expression_type.index){
                case UMINUS_E:
                    break;
                    
                case NOT_E:
                    break;
                    
                default:
                    break;
            }
        }
        else if(root->n_children > 1){
            
            switch(root->expression_type.index)
            {
                case ADD_E: case SUB_E: case DIV_E: case MUL_E:
                    break;
                        
                case  LEQUAL_E: case GEQUAL_E: case GREATER_E: case LESS_E:
                    break;
                        
                case AND_E: case OR_E:
                    break;
                    
                case EQUAL_E: case NEQUAL_E:
                    break;
                        
                case FUNC_CALL_E: 
                    break;
                
                case ARRAY_INDEX_E:
                    break;
                
                default:
                    break;
            }
        }
        
}

data_type_t typecheck_variable(node_t* root){
    
}

data_type_t typecheck_assignment(node_t* root)
{
	if(outputStage == 10){
		printf( "Type checking assignment\n");
	}
}
