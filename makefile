shell: parser.tab.o lex.yy.o ast.o sortlist.o jobs.o exec.o 
	gcc -Wall -o mzsh ast.o sortlist.c jobs.c exec.c parser.tab.o lex.yy.o -lfl -lreadline

parser.tab.c: parser.y
	bison -d parser.y

parser.tab.o: parser.tab.c
	gcc -c parser.tab.c

lex.yy.c: lexer.l
	flex lexer.l

lex.yy.o: lex.yy.c
	gcc -c lex.yy.c

ast.o: ast.c
	gcc -c ast.c

sortlist.o: sortlist.c
	gcc -c sortlist.c

jobs.o: jobs.c
	gcc -c jobs.c

exec.o: exec.c
	gcc -c exec.c

clean:
	rm parser.tab.c parser.tab.h parser.tab.o lex.yy.c lex.yy.o ast.o sortlist.o jobs.o exec.o
