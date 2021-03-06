#include "generator.h"
#include "optimizer.h"
#include "nodetypes.h"
#include "symtab.h"
#include <assert.h>
#include <sys/types.h>
extern int outputStage;    // This variable is located in vslc.c
static char* currentClass = NULL;
int peephole = 1;

/* Registers */
// stackpointer = r13 = sp, framepointer = r11 / r7 = fp, linkregister (return address) = r14 = lr
static char *lr = "lr", *r0 = "r0", *r1 = "r1", *r2 = "r2", *r3 = "r3", *fp =
		"fp", *sp = "sp", *r5 = "r5", *r6 = "r6", *d0 = "d0", *d1 = "d1", *s0 =
		"s0", *s1 = "s1", *pc = "pc";

/* Start and last element for emitting/appending instructions */
static instruction_t *start = NULL, *last = NULL;

/* Support variables for nested while, for, if and continue.*/
node_t *continue_target;
char *continue_target_label;
int continue_target_depth;
static int while_count = 0;
static int for_count = 0;
static int if_count = 0;

// The counter used for the debug printing.
static int nodeCounter = 0;

/* Provided auxiliaries... */

static void instruction_append(instruction_t *next) {
	if (start != NULL) {
		last->next = next;
		last = next;
	}
	else
		start = last = next;
}

static void instruction_add(opcode_t op, char *arg1, char *arg2, int off1,
		int off2) {
	instruction_t *i = (instruction_t *) malloc(sizeof(instruction_t));
	i->opcode = op;
	i->offsets[0] = off1;
	i->offsets[1] = off2;
	i->operands[0] = arg1;
	i->operands[1] = arg2;
	i->next = NULL;
	instruction_append(i);
}

static void instruction_add3(opcode_t op, char* arg1, char* arg2, char* arg3) {
	instruction_t *i = (instruction_t *) malloc(sizeof(instruction_t));
	i->opcode = op;
	i->offsets[0] = 0;
	i->offsets[1] = 0;
	i->operands[0] = arg1;
	i->operands[1] = arg2;
	i->operands[2] = arg3;
	i->next = NULL;
	instruction_append(i);
}

static void instructions_finalize(void) {
}
;

/*
 * Smart wrapper for "printf". 
 * Makes a comment in the assembly to guide.
 * Also prints a copy to the debug stream if needed.
 */
static void tracePrint(const char * string, ...) {
	va_list args;
	char buff[1000];
	char buff2[1000];
	// 
	va_start(args, string);
	vsprintf(buff, string, args);
	va_end(args);

	sprintf(buff2, "%d %s", nodeCounter++, buff);

	if (outputStage == 10)
		fprintf(stderr, "%s", buff2);

	instruction_add(COMMMENT, STRDUP(buff2), NULL, 0, 0);
}
void gen_ARRAY_INDEX_rvalue(node_t *root);
void gen_default(node_t *root, int scopedepth) {
	/* Everything else can just continue through the tree */
	if (root == NULL) {
		return;
	}

	for (int i = 0; i < root->n_children; i++)
		if (root->children[i] != NULL)
			root->children[i]->generate(root->children[i], scopedepth);
}
void gen_SUB_tree(node_t *root, int scopedepth){
	if (root == NULL) {
			return;
	}
	gen_default(root, scopedepth);
	root->generate(root, scopedepth);
}
void gen_PROGRAM(node_t *root, int scopedepth) {
	/* Output the data segment */
	if (outputStage == 12)
		strings_output( stdout);
	instruction_add(STRING, STRDUP(".text"), NULL, 0, 0);

	tracePrint("Starting PROGRAM\n");

	gen_default(root, scopedepth);    //RECUR();

	print_start();

	/**
	 * should only have one functionlist node after simplification
	 * the first function node should be root->children[0]->children[0].
	 * But hasn't the root node already traversed all its children
	 * on line 108??
	 */
	tracePrint("hello I am here PROGRAM\n");
	assert(root->n_children == 1);
	node_t *node_ptr = root->children[0];
	if (node_ptr != NULL) {
		if (node_ptr->children[0] != NULL) {
			char func_label[50] = {0};
			sprintf(func_label, "_%s", node_ptr->children[0]->function_entry->label);
			/* caller saves registers on stack */
			//instruction_add(STRING, STRDUP("\tpush {fp, lr}"), NULL, 0, 0);
			/* branch and link */
			instruction_add(BL, STRDUP(func_label), NULL, 0, 0);
			//instruction_add(STRING, STRDUP("\tpop {fp, pc}"), NULL, 0, 0);
		}
	}
	tracePrint("End PROGRAM\n");

	print_end();

	if (outputStage == 12)
		instructions_print( stdout);
	instructions_finalize();
}
void gen_debug_print(void){
	tracePrint("Starting gen_debug_print\n");
	instruction_add(STRING, STRDUP("\tpush \t{r0-r11, lr}"), NULL, 0, 0);
	instruction_add(PUSH, r0, NULL, 0, 0);
	instruction_add(PUSH, r1, NULL, 0, 0);
	instruction_add(PUSH, r2, NULL, 0, 0);
	instruction_add(PUSH, r3, NULL, 0, 0);
	/* parameter preparation */
	instruction_add(PUSH, r6, NULL, 0, 0);
	instruction_add(POP, r6, NULL, 0, 0);
	char const2str[50]= {0};
	//sprintf(const2str, "#0x%X", (int)(STRDUP(name)));/* address of printing name */
	instruction_add(MOVE32, r0, STRDUP("string_of_gen_debug_print"), 0, 0);
	instruction_add(BL, STRDUP("printf"), NULL, 0, 0);
	// register value printing

	for (int i = 0; i < 4; ++i) {
		//sprintf(const2str, "#0x%X", (int)(STRDUP(name)));
		instruction_add(STRING, STRDUP("\tmovw  r0, #:lower16:.INTEGER"),
				NULL, 0, 0);
		instruction_add(STRING, STRDUP("\tmovt  r0, #:upper16:.INTEGER"),
				NULL, 0, 0);
		instruction_add(POP, r1, NULL, 0, 0);/* r1, is the right parameter list */
		instruction_add(BL, STRDUP("printf"), NULL, 0, 0);
	}

	/* finished */
	instruction_add(MOVE32, r0, STRDUP("0x0A"), 0, 0);
	instruction_add(BL, STRDUP("putchar"), NULL, 0, 0);
	/* recover stack */
	instruction_add(STRING, STRDUP("\tpop \t{r0-r11, lr}"), NULL, 0, 0);
	tracePrint("Ending gen_debug_print\n");
}
void gen_debug_mem_print(void){
	/* get the argument: address of mem */
	//instruction_add(LDR, r0, fp, 0, 0);
	//gen_debug_print();
	instruction_add(MOV, r0, r3, 0, 0);
	//instruction_add(BL, STRDUP("debugprint_r0"), NULL, 0, 0);
	//gen_debug_print();
	/**
	 * now r0 is address
	 */
	instruction_add(LDR, r0, r0, 0, 0);
	instruction_add(BL, STRDUP("debugprint_r0"), NULL, 0, 0);
}
void gen_FUNCTION(node_t *root, int scopedepth) {
	scopedepth++;
	tracePrint("Starting FUNCTION (%s) with depth %d\n", root->label,
			scopedepth);
	/**
	 * function
	 *	--parameter_list
	 *	--statement_list
	 */
	if(strcmp(root->function_entry->label, "debug_registers_print")){
		/* make label */
		instruction_add(LABEL, root->function_entry->label, NULL, 0, 0);
		/* set up stack frame */
		instruction_add(PUSH, lr, NULL, 0, 0);
		instruction_add(PUSH, fp, NULL, 0, 0);
		instruction_add(MOV, fp, sp, 0, 0);
		/* generate code for body of function */
		assert(root->n_children == 2);
		gen_default(root->children[1], scopedepth);/*!< bypass parameter_list node, avoid stack redecalration*/
		/* special code injection */
//		if(strcmp(root->function_entry->label, "debug_mem_print")){
//			gen_debug_mem_print();
//		}
		/* remove stack frame, jump to retrun address */
		instruction_add(MOV, sp, fp, 0, 0);
		instruction_add(POP, fp, NULL, 0, 0);
		instruction_add(POP, pc, NULL, 0, 0);
	}else{
		/* generate special debugging functions */
		/* make label */
		instruction_add(LABEL, root->function_entry->label, NULL, 0, 0);
		/* set up stack frame */
		instruction_add(PUSH, lr, NULL, 0, 0);
		instruction_add(PUSH, fp, NULL, 0, 0);
		instruction_add(MOV, fp, sp, 0, 0);
		/* generate code for body of function */
		gen_debug_print();
		/* remove stack frame, jump to retrun address */
		instruction_add(MOV, sp, fp, 0, 0);
		instruction_add(POP, fp, NULL, 0, 0);
		instruction_add(POP, pc, NULL, 0, 0);
	}
	tracePrint("Leaving FUNCTION (%s) with depth %d\n", root->label,
			scopedepth);
	scopedepth--;
}
/**
 *
 * @param size in 4bytes
 * @return on top of stack
 */
void gen_malloc( unsigned int size_in_words){
	/* caller saves registers on stack */
	instruction_add(STRING, STRDUP("\tpush\t{r1-r11}"), NULL, 0, 0);
	/* caller pushes parameters on stack */
	char const2str[50] = {0};
	sprintf(const2str, "#%d", size_in_words*4);
	instruction_add(MOVE32, r3, STRDUP(const2str), 0, 0);
	instruction_add(PUSH, r3, NULL, 0, 0);
	/* caller jumps to called function address */
	instruction_add(BL, STRDUP("_malloc"), NULL, 0, 0);
	/* caller removes parameters, restores registers */
	sprintf(const2str, "#%d", 4);
	instruction_add3(ADD, sp, sp, STRDUP(const2str));
	instruction_add(STRING, STRDUP("\tpop\t{r1-r11}"), NULL, 0, 0);
	/* use results, push the value/ptr on top of stack, assuming return value on r0 */
	instruction_add(PUSH, r0, NULL, 0, 0);
}
void gen_ARRAY(int nDimensions, int* dimensions) {
	/* generate myself */
	gen_malloc(dimensions[0]);
    if(nDimensions == 1){
    	// finished
    	return;
    }else if(nDimensions < 1){
    	assert(false);
    }
    /* nDimensions > 1*/
    //instruction_add(POP, r2, NULL, 0, 0);  /* r2 <= preivious dimension array head */
    //! why pop failed here?
    for (int i = 0; i < dimensions[0]; ++i) {
    	//instruction_add(STRING, STRDUP("\tpush\t{r1-r11}"), NULL, 0, 0);
    	gen_ARRAY(nDimensions-1, dimensions+1);
    	//instruction_add(STRING, STRDUP("\tpop\t{r1-r11}"), NULL, 0, 0);
		/* assign the newly allocated to proper position */
		instruction_add(POP, r0, NULL, 0, 0); /* r0 <= new allocate */
		instruction_add(POP, r2, NULL, 0, 0);  /* r2 <= preivious dimension array head */
		instruction_add(STR, r0, r2, 0, 4*i); /* [r2 + 4i] <= r0 */
		instruction_add(PUSH, r2, NULL, 0, 0); /* put old address on top of stack */
	}
}
void gen_DECLARATION_STATEMENT(node_t *root, int scopedepth) {
	scopedepth++;
	tracePrint("Starting DECLARATION: adding space on stack\n");
	tracePrint("\t DECLARATION: %s\n", root->label);
	/**
	 * after simplification, the layout should be declaration-{data_type,label}
	 * WITHOUT ANY CHILDREN
	 */
	assert(root);
	assert(root->n_children == 0);
	/* all base data types and array pointers are 4 bytes long */
	instruction_add3(SUB, sp, sp, "#4");
	tracePrint("Ending DECLARATION\n");
	scopedepth--;
}
/**
 * generation code for index_node address calculation
 * @param root root if index_node tree
 * @return the address/result will push on top of stack
 */
void gen_ARRAY_INDEX_e_address_calculation(node_t *root){
	assert(root);
	assert(root->expression_type.index == ARRAY_INDEX_E);
	/**
	 * var[X][Y][Z]
	 *
	 * idx-expr(Z)
	 * 	\idx-expr(Y)
	 * 	  \idx-expr(X)
	 * 	    \variable(var)
	 *
	 * 	now expr := INT_CONST, NOT YET including arithmetic expressions
	 */
	if(root->children[0]->nodetype.index != variable_n.index){
		gen_ARRAY_INDEX_e_address_calculation(root->children[0]);
		/*
		 * now r0 and stack top stores the address of var[X]/left child
		 */
		if(root->children[1]->expression_type.index == ARRAY_INDEX_E){
			/* nested index expressions cases*/
			gen_ARRAY_INDEX_rvalue(root->children[1]);
		}else{
			gen_SUB_tree(root->children[1], 0);// generate Y
		}

		instruction_add(POP, r2, NULL, 0, 0); // r2 <= Y
		instruction_add(POP, r3, NULL, 0, 0); // r3 <= left idx's address
		/* fetch left idx's value, which is still address of head of next dimension array*/
		instruction_add(LDR, r3, r3, 0, 0);// r3 <= [r3]
		instruction_add3(LSL, r2, r2, STRDUP("#2"));// r2 < 4*r2
		instruction_add3(ADD, r0, r3, r2);// r0 <= r3 + r2
		/* now r0 stores the address of var[X][Y] */
		/* try to protect r0 */
		instruction_add(PUSH, r0, NULL, 0, 0);
	}else{
		assert(root->children[0]->nodetype.index == variable_n.index);
		gen_VARIABLE(root->children[0], 0);// generate var

		if(root->children[1]->nodetype.index == expression_n.index){
			/* nested index expressions cases*/
			//gen_ARRAY_INDEX_rvalue(root->children[1]);
			gen_EXPRESSION(root->children[1], 0);
		}else{
			gen_SUB_tree(root->children[1], 0);// generate X
		}

		instruction_add(POP, r2, NULL, 0, 0); // r2 <= X
		instruction_add(POP, r3, NULL, 0, 0); // r3 <= var
		instruction_add3(LSL, r2, r2, STRDUP("#2"));// r2 < 4*r2
		instruction_add3(ADD, r0, r3, r2);// r0 <= r3 + r2
		/* now r0 stores the address of var[X] */
		/* try to protect r0 */
		instruction_add(PUSH, r0, NULL, 0, 0);
	}

}
/**
 *
 * @param root
 * @return expression value will be put on top of stack
 */
void gen_ARRAY_INDEX_rvalue(node_t *root){
	assert(root);
	assert(root->expression_type.index == ARRAY_INDEX_E);
	gen_ARRAY_INDEX_e_address_calculation(root);
	instruction_add(POP, r0, NULL, 0, 0); // r0 <= idx's address
	instruction_add(LDR, r0, r0, 0, 0);
	instruction_add(PUSH, r0, NULL, 0, 0); // push on top of stack
}
void gen_EXPRESSION(node_t *root, int scopedepth) {
	tracePrint("Starting EXPRESSION of type %s\n",
			(char*) root->expression_type.text);
	/** the expressions possibly involved in assignment statement **/
	/** exclude the const/variable such more primary ones **/
	/* function call, array access, new, what about arithmetic expresions? */
	/* these have side effects as well as can be treated as evaluable expressions */
	assert(root);
	//gen_default(root, scopedepth);
	switch (root->expression_type.index) {
		/**
		 * call ::= variable ’(’ argument list ’)’
		 */
		/* left child 'variable' doesn't have symbol entry
		 * , can't be applied with gen_VARIABLE
		 * , it exists in register r0 as return value */
		case FUNC_CALL_E:
		{
			char const2str[50] = {0};
			node_t *arg_list = NULL;
			assert(root->children[0]);
			assert(root->n_children >= 1);
			if(root->children[1]){
				/* may don't have arguments */
				arg_list = root->children[1];
			}

			/* caller saves registers on stack */
			instruction_add(STRING, STRDUP("\tpush \t{r1-r11}"), NULL, 0, 0);
			/* caller pushes parameters on stack */
			gen_default(arg_list, scopedepth); /* generate arg_list's code */
				/* in gen_variable/constant already push on top of stack !!!!!*/
				/* argument_list is actually expression_list, could be constants or other composition other than variables */

			/* caller saves return address in link register*/
				/* use bl to automate this link register's saving */
			/* caller jumps to called function address */
			char func_label[50] = {0};
			sprintf(func_label, "_%s", root->function_entry->label);
			instruction_add(BL, STRDUP(func_label), NULL, 0, 0);
			/* block until return from callee */

			/* caller removes parameters, restores registers */
			/* all parameters are 4 bytes long, just increase sp */
			if(root->children[1]){/** MAY HAVE ZERO ARGUMENTS */
				sprintf(const2str, "#%d", 4*(arg_list->n_children));
				instruction_add3(ADD, sp, sp, STRDUP(const2str));
			}
			instruction_add(STRING, STRDUP("\tpop \t{r1-r11}"), NULL, 0, 0);
			/* use results, push the value on top of stack, assuming return value on r0 */
			instruction_add(PUSH, r0, NULL, 0, 0);
		}
		break;
		/**
		 * NEW type
		 */
		case NEW_E:
		{
			/**
			 * simplified node of array type
			 * -data_type.base = ARRAY_TYPE
			 * -data_type.array_type = INT_TYPE
			 * -data_type.n_dimensions = 2
			 * -data_type.dimensions = [3,3]
			 */
			assert(root->n_children==1);
			assert(root->children[0]);
			if(root->children[0]->data_type.base_type == ARRAY_TYPE){
				/* ARRAY TYPE OF any four bytes long */
				gen_ARRAY(root->children[0]->data_type.n_dimensions,
						root->children[0]->data_type.dimensions);
			}else{
				//possible class/type dynamic allocation
			}
		}
		break;
		/**
		 * \brief Array access
		 * lvalue := expr '[' expr ']'
		 */
		case ARRAY_INDEX_E:
		{
			/**
			 * idx-expr
			 * 	\idx-expr
			 * 	  \...
			 * 	    \variable
			 *
			 * 	now expr := INT_CONST, NOT YET including arithmetic expressions
			 */
			/** TODO why not flatten/simplify array_index_expression? */
			assert(root->n_children==2);
			assert(root->children[0]);
			gen_ARRAY_INDEX_rvalue(root);
		}
		break;
		default:
			gen_default(root, scopedepth);
		break;
	}
	/* do we need to pop children then push myself? assuming children already pushed their value */
	tracePrint("Ending EXPRESSION of type %s\n",
			(char*) root->expression_type.text);
}

void gen_VARIABLE(node_t *root, int scopedepth) {
	scopedepth++;
	tracePrint("Starting VARIABLE\n");
	assert(root);

	if(root->entry){
	/* bypass the function variable node */
		if(root->entry->stack_offset>0){
			/* local variables or arguments */
			tracePrint("\tI am an argument passing to a function!\n");
		}
		instruction_add(LDR, r3, fp, 0, root->entry->stack_offset);
		/* push the value on top of stack for possible assignement, as rhs value/constant ? */
		/* or just for evaluating nesting arithmetic/logic expressions */
		instruction_add(PUSH, r3, NULL, 0, 0);

		tracePrint("End VARIABLE %s, depth difference: %d, stack offset: %d\n",
			root->label, 0, root->entry->stack_offset);
	} else {
		tracePrint("End VARIABLE %s, depth difference: %d",
					root->label, 0);
	}
	scopedepth--;
}
typedef union ufloat {
	float f;
	uint32_t u;
} ufloat_t;
void gen_CONSTANT(node_t * root, int scopedepth) {
	scopedepth++;// keep compiler happy
	tracePrint("Starting CONSTANT\n");
	assert(root);
	//assert(root->nodetype.index == CONSTANT);
	char const2str[50] = {0};
	switch (root->data_type.base_type) {
		case INT_TYPE:
		{
			tracePrint("\tINT\n");
			int32_t value = (int32_t)root->int_const;
			sprintf(const2str,"#%d",value);
			instruction_add(MOVE32, r3, STRDUP(const2str), 0, 0);
		}
		break;
		case STRING_TYPE:
		{
			tracePrint("\tSTRING\n");
			int32_t value = (int32_t)root->string_index;
			sprintf(const2str,"#.STRING%d",value);
			instruction_add(MOVE32, r3, STRDUP(const2str), 0, 0);
		}
		break;
		case BOOL_TYPE:
		{
			tracePrint("\tBOOL\n");
			int32_t value = (int32_t)root->bool_const;
			sprintf(const2str,"#%d",value);
			instruction_add(MOV, r3, STRDUP(const2str), 0, 0);
		}
		break;
		case FLOAT_TYPE:
		{
			tracePrint("\tFLOAT\n");
			ufloat_t value;
			value.f = root->float_const;
			sprintf(const2str,"#0x%X",value.u);
			instruction_add(MOVE32, r3, STRDUP(const2str), 0, 0);
		}
		break;
		default:
		break;
	}
	instruction_add(PUSH, r3, NULL, 0, 0);
	tracePrint("End CONSTANT\n");
	scopedepth--;
}

void gen_ASSIGNMENT_STATEMENT(node_t *root, int scopedepth) {
	tracePrint("Starting ASSIGNMENT_STATEMENT\n");
	/**
	 * assignment statement ::= lvalue ASSIGN expression
	 */
	assert(root);
	assert(root->n_children == 2);
	assert(root->nodetype.index == assignment_statement_n.index);
	tracePrint("#####  right child of assign#####\n");
	gen_SUB_tree(root->children[1], scopedepth);
	tracePrint("#####  end right child of assign#####\n");


	/* store rvalue in address of lhs*/
	if(root->children[0]->expression_type.index != array_index_e.index){
		/** common lvalue expression */
		/* acquire rvalue */
		instruction_add(POP, r5, NULL, 0, 0);
		assert(root->children[0]->entry);
		/* STORE to lvalue's address */
		instruction_add(STR, r5, fp, 0, root->children[0]->entry->stack_offset);

	}else{
		/**
		 *  lvalue is array_index_expression
		 */
		gen_ARRAY_INDEX_e_address_calculation(root->children[0]);
		/* acquire rvalue */
		instruction_add(POP, r0, NULL, 0, 0); /*! address */
		instruction_add(POP, r5, NULL, 0, 0); /*! rvalue */
		instruction_add(STR, r5, r0, 0, 0);
	}
	tracePrint("End ASSIGNMENT_STATEMENT\n");
}

void gen_RETURN_STATEMENT(node_t *root, int scopedepth) {
	tracePrint("Starting RETURN_STATEMENT\n");
	/**
	 * RETURN expression
	 */
	assert(root);
	assert(root->nodetype.index == return_statement_n.index);
	assert(root->n_children==1);
	assert(root->children[0]);
	//assert(root->children[0]->nodetype.index == expression_n.index );// could be variable, simplified
	gen_default(root, scopedepth);
	/* now the expression's value already pushed on top of stack if gen_EXPRESSION works as expect */
	/* pop stack top to r0, which used as return value */
	instruction_add(POP, r0, NULL, 0, 0);
	tracePrint("End RETURN_STATEMENT\n");
}

void gen_PRINT_STATEMENT(node_t* root, int scopedepth) {
	tracePrint("Starting PRINT_STATEMENT\n");

	instruction_add(PUSH, r6, NULL, 0, 0);
	instruction_add(POP, r6, NULL, 0, 0);

	for (int i = 0; i < root->children[0]->n_children; i++) {

		root->children[0]->children[i]->generate(root->children[0]->children[i],
				scopedepth);

		//Pushing the .INTEGER constant, which will be the second argument to printf,
		//and cause the first argument, which is the result of the expression, and is
		//allready on the stack to be printed as an integer
		base_data_type_t t = root->children[0]->children[i]->data_type.base_type;
		switch (t) {
		case INT_TYPE:
			instruction_add(STRING, STRDUP("\tmovw  r0, #:lower16:.INTEGER"),
					NULL, 0, 0);
			instruction_add(STRING, STRDUP("\tmovt  r0, #:upper16:.INTEGER"),
					NULL, 0, 0);
			instruction_add(POP, r1, NULL, 0, 0);
			break;

		case FLOAT_TYPE:
			instruction_add(LOADS, sp, s0, 0, 0);
			instruction_add(CVTSD, s0, d0, 0, 0);
			instruction_add(STRING, STRDUP("\tfmrrd r2, r3, d0"), NULL, 0, 0);
			instruction_add(STRING, STRDUP("\tmovw  r0, #:lower16:.FLOAT"),
					NULL, 0, 0);
			instruction_add(STRING, STRDUP("\tmovt  r0, #:upper16:.FLOAT"),
					NULL, 0, 0);

			// And now the tricky part... 8-byte stack alignment :(
			// We have at least 4-byte alignment always.
			// Check if its only 4-byte aligned right now by anding that bit in the stack-pointer.
			// Store the answer in r5, and set the zero flag.
			instruction_add(STRING, STRDUP("\tandS  r5, sp, #4"), NULL, 0, 0);
			// Now use the zero flag as a condition to optionally change the stack-pointer
			instruction_add(STRING, STRDUP("\tpushNE        {r5}"), NULL, 0, 0);
			break;

		case BOOL_TYPE:
			instruction_add(POP, r1, NULL, 0, 0);
			instruction_add(MOV, r0, "#0", 0, 0);
			instruction_add(CMP, r0, r1, 0, 0);
			instruction_add(MOV, r1, "#0", 0, 0);
			instruction_add(MOVNE, r1, "#1", 0, 0);

			instruction_add(STRING, STRDUP("\tmovw  r0, #:lower16:.INTEGER"),
					NULL, 0, 0);
			instruction_add(STRING, STRDUP("\tmovt  r0, #:upper16:.INTEGER"),
					NULL, 0, 0);

			break;

		case STRING_TYPE:
			instruction_add(POP, r0, NULL, 0, 0);
			break;

		default:
			//instruction_add(PUSH, STRDUP("$.INTEGER"), NULL, 0, 0);
			instruction_add(STRING, STRDUP("\tmovw  r0, #:lower16:.INTEGER"),
					NULL, 0, 0);
			instruction_add(STRING, STRDUP("\tmovt  r0, #:upper16:.INTEGER"),
					NULL, 0, 0);
			instruction_add(POP, r1, NULL, 0, 0);
			fprintf(stderr,
					"WARNING: attempting to print something not int, float or bool\n");
			break;
		}

		instruction_add(BL, STRDUP("printf"), NULL, 0, 0);

		// Undo stack alignment.
		if (t == FLOAT_TYPE) {
			// Redo the zero flag test on r5, as it will give the same answer as the first test on sp.
			instruction_add(STRING, STRDUP("\tandS  r5, #4"), NULL, 0, 0);
			// Conditionally remove the alignment.
			instruction_add(STRING, STRDUP("\tpopNE {r5}"), NULL, 0, 0);
		}
	}

	instruction_add(MOVE32, r0, STRDUP("0x0A"), 0, 0);
	instruction_add(BL, STRDUP("putchar"), NULL, 0, 0);

	tracePrint("Ending PRINT_STATEMENT\n");
}

static void instructions_print(FILE *stream) {
	instruction_t *this = start;

	while (this != NULL) {
		switch (this->opcode)    // ARM
		{
		case PUSH:
			fprintf(stream, "\tpush\t{%s}\n", this->operands[0]);
			break;

		case POP:
			fprintf(stream, "\tpop\t{%s}\n", this->operands[0]);
			break;

		case MOVE32:
			fprintf(stream, "\tmovw\t%s, #:lower16:%s\n", this->operands[0],
					this->operands[1]);
			fprintf(stream, "\tmovt\t%s, #:upper16:%s\n", this->operands[0],
					this->operands[1]);
			break;

		case MOV:
			fprintf(stream, "\tmov\t%s, %s\n", this->operands[0],
					this->operands[1]);
			break;

		case LDR:
			if (this->offsets[0] == 0 && this->offsets[1] == 0)
				fprintf(stream, "\tldr\t%s, [%s]\n", this->operands[0],
						this->operands[1]);
			else if (this->offsets[0] == 0 && this->offsets[1] != 0)
				fprintf(stream, "\tldr\t%s, [%s, #%d]\n", this->operands[0],
						this->operands[1], this->offsets[1]);
			else if (this->offsets[0] != 0 && this->offsets[1] == 0)
				fprintf(stream, "ERROR, LOAD format not correct\n");
			break;

		case LOADS:
			if (this->offsets[0] == 0 && this->offsets[1] == 0)
				fprintf(stream, "\tflds\t%s, [%s]\n", this->operands[1],
						this->operands[0]);
			else if (this->offsets[0] != 0 && this->offsets[1] == 0)
				fprintf(stream, "\tflds\t%s, [%s, #%d]\n", this->operands[1],
						this->operands[0], this->offsets[0]);
			else if (this->offsets[0] == 0 && this->offsets[1] != 0)
				fprintf(stream, "ERROR, LOAD format not correct\n");
			break;

		case STR:
			if (this->offsets[0] == 0 && this->offsets[1] == 0)
				fprintf(stream, "\tstr\t%s, [%s]\n", this->operands[0],
						this->operands[1]);
			else if (this->offsets[1] != 0 && this->offsets[0] == 0)
				fprintf(stream, "\tstr\t%s, [%s, #%d]\n", this->operands[0],
						this->operands[1], this->offsets[1]);
			else if (this->offsets[0] == 0 && this->offsets[1] != 0)
				fprintf(stream, "ERROR, STORE format not correct\n");
			break;

		case STORES:
			if (this->offsets[0] == 0 && this->offsets[1] == 0)
				fprintf(stream, "\tfsts\t%s, [%s]\n", this->operands[0],
						this->operands[1]);
			else if (this->offsets[0] != 0 && this->offsets[1] == 0)
				fprintf(stream, "\tfsts\t%s, [%s, #%d]\n", this->operands[0],
						this->operands[1], this->offsets[0]);
			else if (this->offsets[0] == 0 && this->offsets[1] != 0)
				fprintf(stream, "ERROR, STORE format not correct\n");
			break;

		case MOVES:
			if (this->offsets[0] == 0 && this->offsets[1] == 0)
				fprintf(stream, "\tmfcpys\t%s, %s\n", this->operands[1],
						this->operands[0]);
			else if (this->offsets[0] != 0 && this->offsets[1] == 0)
				fprintf(stream,
						"\tError: NOT possible for ARM, use load/store\t%d(%s),%s\n",
						this->offsets[0], this->operands[0], this->operands[1]);
			else if (this->offsets[0] == 0 && this->offsets[1] != 0)
				fprintf(stream,
						"\tError: NOT possible for ARM, use load/store\t%s,%d(%s)\n",
						this->operands[0], this->offsets[1], this->operands[1]);
			break;

		case MOVED:
			if (this->offsets[0] == 0 && this->offsets[1] == 0)
				fprintf(stream, "\tmfcpyd TODO\t%s,%s\n", this->operands[1],
						this->operands[0]);
			else if (this->offsets[0] != 0 && this->offsets[1] == 0)
				fprintf(stream,
						"\tError: NOT possible for ARM, use load/store\t%s, [%s,#%d]\n",
						//this->offsets[0], this->operands[0], this->operands[1]   "\ldr\t%d(%s),%s\n",
						this->operands[1], this->operands[0], this->offsets[0]);
			else if (this->offsets[0] == 0 && this->offsets[1] != 0)
				fprintf(stream,
						"\tError: NOT possible for ARM, use load/store\t%s,%d(%s)\n",
						this->operands[0], this->offsets[1], this->operands[1]);
			break;

		case CVTSD:
			if (this->offsets[0] == 0 && this->offsets[1] == 0)
				fprintf(stream, "\tfcvtds\t%s,%s\n", this->operands[1],
						this->operands[0]);
			else if (this->offsets[0] != 0 && this->offsets[1] == 0)
				fprintf(stream, "\tfcvtds TODO\t%d(%s),%s\n", this->offsets[0],
						this->operands[0], this->operands[1]);
			else if (this->offsets[0] == 0 && this->offsets[1] != 0)
				fprintf(stream, "\tfcvtds TODO\t%s,%d(%s)\n", this->operands[0],
						this->offsets[1], this->operands[1]);
			break;

		case ADD:
			if (this->operands[2] == NULL) {
				//Legacy support
				fprintf(stream, "\tadd\t%s, %s\n", this->operands[1],
						this->operands[0]);
			}
			else {
				fprintf(stream, "\tadd\t%s, %s, %s\n", this->operands[0],
						this->operands[1], this->operands[2]);
			}
			break;

		case FADD:
			if (this->offsets[0] == 0 && this->offsets[1] == 0)
				fprintf(stream, "\tfadds\t%s, %s, %s\n", this->operands[1],
						this->operands[1], this->operands[0]);
			break;

		case SUB:
			if (this->operands[2] == NULL)
				fprintf(stream, "\tsub\t%s, %s\n", this->operands[1],
						this->operands[0]);
			else {
				fprintf(stream, "\tsub\t%s, %s, %s\n", this->operands[0],
						this->operands[1], this->operands[2]);
			}
			break;

		case FSUB:
			if (this->offsets[0] == 0 && this->offsets[1] == 0)
				fprintf(stream, "\tfsubs\t%s, %s, %s\n", this->operands[1],
						this->operands[1], this->operands[0]);
			else
				fprintf(stream, "Not supported...\tfsub\t%s, %s\n",
						this->operands[1], this->operands[0]);
			break;

		case MUL:
			fprintf(stream, "\tmul\t%s,%s,%s\n", this->operands[0],
					this->operands[1], this->operands[2]);

			break;
		case FMUL:
			if (this->offsets[0] == 0)
				fprintf(stream, "\tfmuls\t %s, %s, %s\n", this->operands[1],
						this->operands[1], this->operands[0]);
			else
				fprintf(stream, "Not supported...\tfmul\t%d(%s)\n",
						this->offsets[0], this->operands[0]);
			break;
		case DIV:

			fprintf(stream, "\tsdiv\t%s, %s, %s\n", this->operands[0],
					this->operands[1], this->operands[2]);

			break;
		case FDIV:
			if (this->offsets[0] == 0 && this->operands[1] == NULL)
				fprintf(stream, "\tfdivs\ts0, s0, %s\n", this->operands[0]);
			else if (this->offsets[0] == 0)
				fprintf(stream, "\tfdivs\t%s, %s, %s\n", this->operands[1],
						this->operands[1], this->operands[0]);
			else
				fprintf(stream, "\tidivl TODO\t%d(%s)\n", this->offsets[0],
						this->operands[0]);
			break;
		case NEG:
			fprintf(stream, "\tneg\t%s, %s\n", this->operands[0],
					this->operands[1]);
			break;
		case FNEG:
			fprintf(stream, "\tfnegs\t%s, %s\n", this->operands[0],
					this->operands[1]);
			break;
		case CMP:
			if (this->offsets[0] == 0 && this->offsets[1] == 0)
				fprintf(stream, "\tcmp\t%s,%s\n", this->operands[0],
						this->operands[1]);
			break;
		case FCMP:
			fprintf(stream, "\tfcmps\t%s,%s\n", this->operands[0],
					this->operands[1]);
			fprintf(stream, "\tvmrs APSR_nzcv, FPSCR\n");
			break;

		case MOVGT:
			fprintf(stream, "\tmovgt\t %s, %s\n", this->operands[0],
					this->operands[1]);
			break;
		case MOVGE:
			fprintf(stream, "\tmovge\t %s, %s\n", this->operands[0],
					this->operands[1]);
			break;
		case MOVLT:
			fprintf(stream, "\tmovlt\t %s, %s\n", this->operands[0],
					this->operands[1]);
			break;
		case MOVLE:
			fprintf(stream, "\tmovle\t %s, %s\n", this->operands[0],
					this->operands[1]);
			break;
		case MOVEQ:
			fprintf(stream, "\tmoveq\t %s, %s\n", this->operands[0],
					this->operands[1]);
			break;
		case MOVNE:
			fprintf(stream, "\tmovne\t %s, %s\n", this->operands[0],
					this->operands[1]);
			break;

		case BL:
			fprintf(stream, "\tbl\t");
			fprintf(stream, "%s\n", this->operands[0]);
			break;
		case LABEL:
			fprintf(stream, "_%s:\n", this->operands[0]);
			break;

		case B:
			fprintf(stream, "\tb\t%s\n", this->operands[0]);
			break;

		case BEQ:
			fprintf(stream, "\tbeq\t%s\n", this->operands[0]);
			break;
		case BNE:
			fprintf(stream, "\tbne\t%s\n", this->operands[0]);
			break;

		case STRING:
			fprintf(stream, "%s\n", this->operands[0]);
			break;

		case COMMMENT:
			fprintf(stream, "#%s", this->operands[0]);
			break;

		case NIL:
			break;

		case LSL:
			fprintf(stream, "\tlsl\t%s, %s, %s\n", this->operands[0],
					this->operands[1], this->operands[2]);
			break;

		default:
			fprintf( stderr, "Error in instruction stream\n");
			break;
		}
		this = this->next;
	}
}

void print_start() {
	// Start of assemlby program
	// Debug functions and system call wrappers
	instruction_add(STRING, STRDUP("\nstring_of_gen_debug_print: .ascii \"Debug print register is \""), NULL, 0, 0);
	instruction_add(STRING, STRDUP(".ascii \"\\000\"\n"), NULL, 0, 0);
	instruction_add(STRING, STRDUP(".ascii \"\\000\"\n"), NULL, 0, 0);
	instruction_add(STRING, STRDUP(".ascii \"\\000\"\n"), NULL, 0, 0);
	instruction_add(STRING, STRDUP(".ascii \"\\000\"\n"), NULL, 0, 0);
	instruction_add(STRING, STRDUP("debugprint:"), NULL, 0, 0);
	instruction_add(STRING, STRDUP("\tpush {r0-r11, lr}"), NULL, 0, 0);
	instruction_add(STRING, STRDUP("\tmovw	r0, #:lower16:.DEBUG"), NULL, 0, 0);
	instruction_add(STRING, STRDUP("\tmovt	r0, #:upper16:.DEBUG"), NULL, 0, 0);
	instruction_add(BL, STRDUP("printf"), NULL, 0, 0);
	instruction_add(STRING, STRDUP("\tpop {r0-r11, pc}"), NULL, 0, 0);
	instruction_add(STRING, STRDUP("debugprint_r0:"), NULL, 0, 0);
	instruction_add(STRING, STRDUP("\tpush {r0-r11, lr}"), NULL, 0, 0);
	instruction_add(MOV, r1, r0, 0, 0);
	instruction_add(STRING, STRDUP("\tmovw	r0, #:lower16:.DEBUGINT"), NULL, 0,
			0);
	instruction_add(STRING, STRDUP("\tmovt	r0, #:upper16:.DEBUGINT"), NULL, 0,
			0);
	instruction_add(BL, STRDUP("printf"), NULL, 0, 0);
	instruction_add(STRING, STRDUP("\tpop {r0-r11, pc}"), NULL, 0, 0);
	instruction_add(STRING, STRDUP("_malloc:"), NULL, 0, 0);
	instruction_add(PUSH, lr, NULL, 0, 0);
	instruction_add(PUSH, fp, NULL, 0, 0);
	instruction_add(LDR, r0, sp, 0, 8);
	instruction_add(BL, STRDUP("malloc"), NULL, 0, 0);
	instruction_add(POP, fp, NULL, 0, 0);
	instruction_add(STRING, STRDUP("\tpop {pc}"), NULL, 0, 0);

	// Setting up command line arguments
	instruction_add(STRING, STRDUP("main:"), NULL, 0, 0);
	instruction_add(PUSH, lr, NULL, 0, 0);
	instruction_add(PUSH, fp, NULL, 0, 0);
	instruction_add(MOV, fp, sp, 0, 0);
	instruction_add(MOV, r5, r0, 0, 0);
	instruction_add3(SUB, r5, r5, "#1");
	instruction_add(CMP, r5, "#0", 0, 0);
	instruction_add(BEQ, STRDUP("noargs"), NULL, 0, 0);
	instruction_add(MOV, r6, r1, 0, 0);
	instruction_add(STRING, STRDUP("pusharg:"), NULL, 0, 0);
	instruction_add(LDR, r0, r6, 0, 4);
	instruction_add3(ADD, r6, r6, STRDUP("#4"));
	instruction_add(MOV, r1, STRDUP("#0"), 0, 0);
	instruction_add(MOV, r2, STRDUP("#10"), 0, 0);
	instruction_add(BL, STRDUP("strtol"), NULL, 0, 0);
	instruction_add(PUSH, r0, NULL, 0, 0);
	instruction_add3(SUB, r5, r5, "#1");
	instruction_add(CMP, r5, "#0", 0, 0);
	instruction_add(BNE, STRDUP("pusharg"), NULL, 0, 0);
	instruction_add(STRING, STRDUP("noargs:"), NULL, 0, 0);
}

void print_end() {
	instruction_add(MOV, sp, fp, 0, 0);
	instruction_add(POP, fp, NULL, 0, 0);
	instruction_add(BL, STRDUP("exit"), NULL, 0, 0);
	instruction_add(STRING, STRDUP(".end"), NULL, 0, 0);
}

