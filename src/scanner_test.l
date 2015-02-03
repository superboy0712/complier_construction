
%{
//#include "parser.h"
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
KEY_WORD	FUNC|PRINT|RETURN|NEW|IF|THEN|ELSE|END|WHILE|DO|END|FOR|TO|INT|BOOL|FLOAT|START|VOID|ARRAY
STRING_CHAR	[^\"\n]
ESP_DQ		\\\"
OPERATORS :=|"+"|"-"|"*"|"/"|">"|"<"|<=|>=|==|!=|"&"{2}|"|"{2}|"!"|"."
%%
{WHITE_SPACE}
{COMMENTS} 	
{KEY_WORD}	{printf( "KEY_W(%s)\n", yytext );}
{OPERATORS} {printf( "OP(%s)\n", yytext );}
{D}+		{printf( "INT(%s)\n", yytext );}
{D}+.{D}+	{printf( "FLOAT(%s)\n", yytext );}
TRUE|FALSE	{printf( "BOOL(%s)\n", yytext );}

\"({STRING_CHAR}|{ESP_DQ})*\"	{ 
				printf( "STR(%s)\n", yytext );
			}
{C}+({D}|{C}|"_")*	{
				printf( "IDENT(%s)\n", yytext );
			}
{PUNCT}		{printf( "PUNCT(%s)\n", yytext );}
.			{printf( "UNMATCHED(%s)\n", yytext );}
%%
int main(){
	yylex();
	return 1;
}
