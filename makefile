shell: lexer.l parser.y ast.c util.c
	bison -d parser.y
	flex lexer.l
	gcc -o mzsh ast.c util.c parser.tab.c lex.yy.c -lfl -lreadline
