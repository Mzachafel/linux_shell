shell: parser.y lexer.l ast.c sortlist.c jobs.c exec.c 
	bison -d parser.y
	flex lexer.l
	gcc -o mzsh ast.c sortlist.c jobs.c exec.c parser.tab.c lex.yy.c -lfl -lreadline
