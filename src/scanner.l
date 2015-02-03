
%{
#include "parser.h"
#include <stdio.h>
extern int outputStage; // This variable is located in vslc.c
    #define RETURN(t) do {                                             \
        if ( outputStage == 1 )                                        \
            printf( "TOKEN ( %d,\t'%s' )\n", t, yytext );    \
        return t;                                                      \
    } while ( 0 )
      
%}

%option array
%option noyywrap
%option yylineno

WHITE_SPACE [ \t\n]+
/* remember .* stands for any string */
D			[0-9]
C			[a-zA-Z]
PUNCT		")"|"("|","|";"|"["|"]"
COMMENTS	"/"{2}.*\n
STRING_CHAR	[^\"\n\\]
ESP_DQ		\\\"
OPERATORS 	"+"|"-"|"*"|"/"|">"|"<"|"!"|"."
%%
{WHITE_SPACE}
{COMMENTS} 	

FUNC		RETURN(FUNC);
PRINT		RETURN(PRINT);
RETURN		RETURN(RETURN);
NEW			RETURN(NEW);
IF			RETURN(IF);
THEN		RETURN(THEN);
ELSE		RETURN(ELSE);
END			RETURN(END);
WHILE		RETURN(WHILE);
DO			RETURN(DO);
FOR			RETURN(FOR);
TO			RETURN(TO);
INT			RETURN(INT);
BOOL		RETURN(BOOL);
FLOAT		RETURN(FLOAT);
START		RETURN(START);
VOID		RETURN(VOID);
ARRAY		RETURN(ARRAY);
STRING		RETURN(STRING);
{OPERATORS}	RETURN((char)yytext[0]); // return the ascii number

:=			RETURN(ASSIGN);
==			RETURN(EQUAL);
>=			RETURN(GEQUAL);



"<""="		RETURN(LEQUAL);
!=			RETURN(NEQUAL);
"&"{2}		RETURN(AND);
"|"{2}		RETURN(OR);

{D}+		RETURN(INT_CONST);
{D}+"."{D}+	RETURN(FLOAT_CONST);
TRUE		RETURN(TRUE_CONST);
FALSE		RETURN(FALSE_CONST);

\"({STRING_CHAR}|{ESP_DQ})*\"	RETURN(STRING_CONST);

{C}+({D}|{C}|"_")*	RETURN(IDENTIFIER);

{PUNCT}		RETURN((char)yytext[0]); // return the ascii number
.			{ 
				fprintf ( stderr, "\tNo rules to match : at line %d"\
				" with yytext: '%s'\n", yylineno, yytext );
			}
%%

