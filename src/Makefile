all: scanner

scanner : lex.yy.c
	clang -std=c99 -Wall -o lexer lex.yy.c
	
lex.yy.c : scanner.l
	flex -o lex.yy.c scanner_test.l

clean :
	rm -f lex.yy*.c lexer*
