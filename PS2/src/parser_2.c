
#include "nodetypes.h"
#include "tree.h"
#include <stdlib.h>

// This defines the type for every $$ value in the productions.
#define YYSTYPE node_t *

#define YYDEBUG 1

// This variable is located in vslc.c
extern int outputStage;

/*
 * Variables connecting the parser to the state of the scanner - defs. will be
 * generated as part of the scanner (lexical analyzer).
 */
extern char yytext[];
extern int yylineno;

/*
 * Wrapper functions for node_init. The main purpose of calling these functions
 * instead of node_init directly is to enable the debug output, as well as a reduction
 * of typing. These functions are named CN for "create
 * node", and L, T, or E if they take an additional label, type or expression_type argument
 * in addition. When the label, type or expression_type is not supplied, node_init is called with
 * default values.
 */
 
node_t* CN(nodetype_t type, int n_children, ...){
	if( outputStage == 2 ) printf( "Hit rule \"%s\" on text '%s' at line %d\n", type.text , yytext, yylineno );
	va_list child_list;
	va_start(child_list, n_children);
	node_t* to_return = node_init(type, NULL, NO_TYPE, default_e, n_children, child_list);
	va_end(child_list);
	return to_return;
}

node_t* CNL(nodetype_t type, char* label, int n_children, ...){
	if( outputStage == 2 ) printf( "Hit rule \"%s\" on text '%s' at line %d\n", type.text , yytext, yylineno );
	va_list child_list;
	va_start(child_list, n_children);
	node_t* to_return = node_init(type, label, NO_TYPE, default_e, n_children, child_list);
	va_end(child_list);
	return to_return;
}

node_t* CNT(nodetype_t type, base_data_type_t base_type, int n_children, ...){
	if( outputStage == 2 ) printf( "Hit rule \"%s\" on text '%s' at line %d\n", type.text , yytext, yylineno );
	va_list child_list;
	va_start(child_list, n_children);
	node_t* to_return = node_init(type, NULL, base_type, default_e, n_children, child_list);
	va_end(child_list);
	return to_return;
}

node_t* CNE(nodetype_t type, expression_type_t expression_type, int n_children, ...){
	if( outputStage == 2 ) printf( "Hit rule \"%s\" on text '%s' at line %d\n", type.text , yytext, yylineno );
	va_list child_list;
	va_start(child_list, n_children);
	node_t* to_return = node_init(type, NULL, NO_TYPE, expression_type, n_children, child_list);
	va_end(child_list);
	return to_return;
}


// Helper for setting the value of an Integer node
static void SetInteger(node_t* node, char *string)
{
	node->int_const = atol ( string );
	node->data_type.base_type= INT_TYPE;
}

// Helper for setting the value of an float node
static void SetFloat(node_t* node, char *string)
{
	node->float_const = atof ( string );
	node->data_type.base_type= FLOAT_TYPE;
}


// Helper for setting the value of an string node
static void SetString(node_t* node, char *string)
{
	node->string_const = STRDUP( string );
	node->data_type.base_type= STRING_TYPE;
}

/*
 * Since the return value of yyparse is an integer (as defined by yacc/bison),
 * we need the top level production to finalize parsing by setting the root
 * node of the entire syntax tree inside its semantic rule instead. This global
 * variable will let us get a hold of the tree root after it has been
 * generated.
 */
node_t *root;


/*
 * These functions are referenced by the generated parser before their
 * definition. Prototyping them saves us a couple of warnings during build.
 */
int yyerror ( const char *error );  /* Defined below */
int yylex ( void );                 /* Defined in the generated scanner */



/* Tokens for all the key words in VSL */


/*
 * Operator precedences: 
 * + and - bind to the left { a+b+c = (a+b)+c }
 * * and / bind left like + and -, but has higher precedence
 * Unary minus has only one operand (and thus no direction), but highest
 * precedence. Since we've already used '-' for the binary minus, unary minus
 * needs a ref. name and explicit setting of precedence in its grammar
 * production: " '-' expression %prec UMINUS "
 */

/*
 * The grammar productions follow below. These are mostly a straightforward
 * statement of the language grammar, with semantic rules building a tree data
 * structure which we can traverse in subsequent phases in order to understand
 * the parsed program. (The leaf nodes at the bottom need somewhat more
 * specific rules, but these should be manageable.)
 * A lot of the work to be done later could be handled here instead (reducing
 * the number of passes over the syntax tree), but sticking to a parser which
 * only generates a tree makes it easier to rule it out as an error source in
 * later debugging.
 */ 
node_t * ss, s1, s2;

void helloworld(void){
//program			: function_list
					{ 
						s1 = CN(function_list_n);
						ss = CN(program_n, 1, s1);
						root = ss;
					}
				;

//function		: type FUNC variable '(' parameter_list ')' START statement_list END	{
						
					}
				;

function_list	: function_list function
					{
						
					}
				| epislon
				;

statement_list	: statement
				| statement_list statement
				;

variable_list	: declaration_statement
				| variable_list ',' expression
				;

expression_list	: expression
				| expression_list ',' expression
				;

parameter_list	: variable_list
				| epislon
				;

argument_list	: expression_list
				| epislon
				;

statement		: declaration_statement ';'
				| assignment_statement ';'
				| if_statement
				| while_statement
				| for_statement
				| print_statement ';'
				| return_statement ';'
				| call ';'
				;

declaration_statement
				: type variable
				;

assignment_statement
				: lvalue ASSIGN expression
				;

if_statement	: IF expression THEN statement_list END
				| IF expression THEN statement_list ELSE statement_list END
				;

while_statement : WHILE expression DO statement_list END
				;

for_statement	: FOR assignment_statement TO expression DO statement_list END
				;

return_statement
				: RETURN expression
				;

print_statement	: PRINT expression_list
				;

expression		: constant
				| expression '+' expression
				| expression '-' expression
				| expression '*' expression
				| expression '/' expression
				| expression '>' expression
				| expression '<' expression
				| expression EQUAL expression
				| expression NEQUAL expression
				| expression GEQUAL expression
				| expression LEQUAL expression
				| expression AND expression
				| expression OR expression
				| UMINUS expression
				| '!' expression
				| NEW type
				| '(' expression ')'
				| call
				| lvalue
				;

call			: variable '(' argument_list ')'
				;

lvalue			: variable
				| expression '[' expression ']'
				;

constant		: TRUE_CONST
				| FALSE_CONST
				| INT_CONST
				| FLOAT_CONST
				| STRING_CONST
				;

type			: INT
				| FLOAT
				| BOOL
				| VOID
				| type ARRAY index_list
				;

index_list		: index_list '[' index ']'
				| '[' index ']'
				;

index			: INT_CONST
				;

variable		: IDENTIFIER
				;

epislon			:
				;
%% 

/*
 * This function is called with an error description when parsing fails.
 * Serious error diagnosis requires a lot of code (and imagination), so in the
 * interest of keeping this project on a manageable scale, we just chuck the
 * message/line number on the error stream and stop dead.
 */
int
yyerror ( const char *error )
{
    fprintf ( stderr, "\tError: %s detected at line %d with yytext: %s\n", error, yylineno, yytext );
    exit ( EXIT_FAILURE );
}
