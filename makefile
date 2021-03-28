shell: lexer.l parser.y main.c ast.c util.c
	bison -d parser.y
	flex lexer.l
	gcc -g -o mzsh ast.c util.c parser.tab.c lex.yy.c main.c -lfl
