%{
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
%}


/* Tokens for all the key words in VSL */
%token INT_CONST FLOAT_CONST TRUE_CONST FALSE_CONST STRING_CONST STRING
%token INT FLOAT BOOL VOID  IDENTIFIER
%token ASSIGN FUNC START PRINT RETURN IF THEN ELSE END WHILE DO
%token EQUAL GEQUAL LEQUAL NEQUAL AND OR  
%token  NEW
%token ARRAY
%token FOR TO

/*
 * Operator precedences: 
 * + and - bind to the left { a+b+c = (a+b)+c }
 * * and / bind left like + and -, but has higher precedence
 * Unary minus has only one operand (and thus no direction), but highest
 * precedence. Since we've already used '-' for the binary minus, unary minus
 * needs a ref. name and explicit setting of precedence in its grammar
 * production: " '-' expression %prec UMINUS "
 */
%nonassoc ARRAY
%nonassoc ']'

%left OR
%left AND
%left EQUAL NEQUAL
%left GEQUAL LEQUAL '<' '>'
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS '!'
%left '[' '.' 

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

%%

program			: function_list 
					{ 
						$$ = CN(program_n, 1, $1);
						root = $$;
					}
				;

function		: type FUNC variable '(' parameter_list ')' START statement_list END	{
						//$$ = CNL(function_n, $1->label, 3, $1, $2, $3); /* trying to access $1->label results in segment fault! */
						$$ = CN(function_n, 3, $1, $2, $3);
					}
				;

function_list	: function_list function
					{
						$$ = CN(function_list_n, 2, $1, $2);
					}
				| epislon
					{
						//$$ = CN(function_list_n, 1, $1);
					}
				;

statement_list	: statement
					{
						$$ = CN(statement_list_n, 1, $1);
					}
				| statement_list statement
					{
						$$ = CN(statement_list_n, 2, $1, $2);
					}
				;

variable_list	: declaration_statement
					{
						$$ = CN(variable_list_n, 1, $1);
					}
				| variable_list ',' expression
					{
						$$ = CN(variable_list_n, 2, $1, $2); 
					}
				;

expression_list	: expression
					{
						$$ = CN(expression_list_n, 1, $1);
					}
				| expression_list ',' expression
					{
						$$ = CN(expression_list_n, 2, $1, $2); 
					}
				;

parameter_list	: variable_list
					{
						$$ = CN(parameter_list_n, 1, $1);
					}
				| epislon
				;

argument_list	: expression_list
					{
						$$ = CN(argument_list_n, 1, $1);
					}
				| epislon
				;

statement		: declaration_statement ';'
					{
						$$ = CN(statement_n, 1, $1);
					}
				| assignment_statement ';'
					{
						$$ = CN(statement_n, 1, $1);
					}
				| if_statement
					{
						$$ = CN(statement_n, 1, $1);
					}
				| while_statement
					{
						$$ = CN(statement_n, 1, $1);
					}
				| for_statement
					{
						$$ = CN(statement_n, 1, $1);
					}
				| print_statement ';'
					{
						$$ = CN(statement_n, 1, $1);
					}
				| return_statement ';'
					{
						$$ = CN(statement_n, 1, $1);
					}
				| call ';'
					{
						$$ = CN(statement_n, 1, $1);
					}
				;

declaration_statement
				: type variable
					{
						$$ = CN(declaration_statement_n, 1, $1);
					}
				;

assignment_statement
				: lvalue ASSIGN expression
					{
						$$ = CN(assignment_statement_n, 2, $1, $2);
					}
				;

if_statement	: IF expression THEN statement_list END
					{
						$$ = CN(if_statement_n, 2, $1, $2);
					}
				| IF expression THEN statement_list ELSE statement_list END
					{
						$$ = CN(if_statement_n, 3, $1, $2, $3);
					}
				;

while_statement : WHILE expression DO statement_list END
					{
						$$ = CN(while_statement_n, 2, $1, $2);
					}
				;

for_statement	: FOR assignment_statement TO expression DO statement_list END
					{
						$$ = CN(for_statement_n, 3, $1, $2, $3);
					}
				;

return_statement
				: RETURN expression
					{
						$$ = CN(return_statement_n, 1, $1);
					}
				;

print_statement	: PRINT expression_list
					{
						$$ = CN(print_statement_n, 1, $1);
					}
				;

expression		: constant
					{
						$$ = CNE(expression_n, constant_e, 1, $1);
					}
				| expression '+' expression
					{
						$$ = CNE(expression_n, add_e, 2, $1, $2);
					}
				| expression '-' expression
					{
						$$ = CNE(expression_n, sub_e, 2, $1, $2);
					}
				| expression '*' expression
					{
						$$ = CNE(expression_n, mul_e, 2, $1, $2);
					}
				| expression '/' expression
					{
						$$ = CNE(expression_n, div_e, 2, $1, $2);
					}
				| expression '>' expression
					{
						$$ = CNE(expression_n, greater_e, 2, $1, $2);
					}
				| expression '<' expression
					{
						$$ = CNE(expression_n, less_e, 2, $1, $2);
					}
				| expression EQUAL expression
					{
						$$ = CNE(expression_n, equal_e, 2, $1, $2);
					}
				| expression NEQUAL expression
					{
						$$ = CNE(expression_n, nequal_e, 2, $1, $2);
					}
				| expression GEQUAL expression
					{
						$$ = CNE(expression_n, gequal_e, 2, $1, $2);
					}
				| expression LEQUAL expression
					{
						$$ = CNE(expression_n, lequal_e, 2, $1, $2);
					}
				| expression AND expression
					{
						$$ = CNE(expression_n, and_e, 2, $1, $2);
					}
				| expression OR expression
					{
						$$ = CNE(expression_n, or_e, 2, $1, $2);
					}
				| UMINUS expression
					{
						$$ = CNE(expression_n, uminus_e, 1, $1);
					}
				| '!' expression
					{
						$$ = CNE(expression_n, not_e, 1, $1);
					}
				| NEW type
					{
						$$ = CNE(expression_n, new_e, 1, $1);
					}
				| '(' expression ')'
					{
						$$ = CNE(expression_n, default_e, 1, $1);
					}
				| call
					{
						$$ = CNE(expression_n, func_call_e, 1, $1);
					}
				| lvalue
					{
						$$ = CNE(expression_n, default_e, 1, $1);
					}
				;

call			: variable '(' argument_list ')'
					{
						$$ = CNE(expression_n, func_call_e, 2, $1, $2); 
					}
				;

lvalue			: variable
					{
						$$ = CNE(expression_n, variable_e, 1, $1);
					}
				| expression '[' expression ']'
					{
						$$ = CNE(expression_n, array_index_e, 2, $1, $2);
					}
				;

constant		: TRUE_CONST
					{
						$$ = CNT(constant_n, )
					}
				| FALSE_CONST
					{
							
					}
				| INT_CONST
					{
							
					}
				| FLOAT_CONST
					{
							
					}
				| STRING_CONST
					{
							
					}
				;

type			: INT
					{
							
					}
				| FLOAT
					{
							
					}
				| BOOL
					{
							
					}
				| VOID
					{
							
					}
				| type ARRAY index_list
					{
							
					}
				;

index_list		: index_list '[' index ']'
					{
							
					}
				| '[' index ']'
					{
							
					}
				;

index			: INT_CONST
					{
							
					}
				;

variable		: IDENTIFIER
					{
							
					}
				;

epislon			:
					{
							
					}
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
