#ifndef AST_H
#define AST_H

typedef struct _arguments {
	char **argv;
	int maxarg;
	int curarg;
} arguments;
	
arguments* creatargs(void);
arguments* addarg(arguments *args, char *arg);
void clearargs(arguments *args);

typedef struct _commands {
	arguments **comv;
	int maxcom;
	int curcom;
} commands;

commands* creatcoms(void);
commands* addcom(commands *coms, arguments *com);
void clearcoms(commands *coms);

#endif
