#include <stdlib.h>
#include <string.h>
#include "ast.h"

#define DEFMAXARGS 8

arguments* creatargs(void)
{
	arguments *args = malloc(sizeof(arguments));
	args->argv = malloc(DEFMAXARGS * sizeof(char *));
	args->maxarg = DEFMAXARGS;
	args->curarg = 0;
	return args;
}

arguments* addarg(arguments *args, char *arg)
{
	if (args->curarg == args->maxarg) {
		if (!arg)
			args->maxarg++;
		else 
			args->maxarg *= 2;
		args->argv = realloc(args->argv, args->maxarg * sizeof(char *));
	}
	args->argv[args->curarg++] = (!arg) ? NULL : strdup(arg);
	return args;
}

void clearargs(arguments *args)
{
	for (int i = 0; i < args->curarg-1; i++)
		free(args->argv[i]);
	free(args);
}



#define DEFMAXCOMS 4

commands* creatcoms(void)
{
	commands* coms = malloc(sizeof(commands));
	coms->comv = malloc(DEFMAXCOMS * sizeof(arguments *));
	coms->maxcom = DEFMAXCOMS;
	coms->curcom = 0;
	return coms;
}

commands* addcom(commands *coms, arguments *com)
{
	if (coms->curcom == coms->maxcom) {
		coms->maxcom *= 2;
		coms->comv = realloc(coms->comv, coms->maxcom * sizeof(arguments *));
	}
	com = addarg(com, NULL);
	coms->comv[coms->curcom++] = com;
	return coms;
}

void clearcoms(commands *coms)
{
	for (int i = 0; i < coms->curcom; i++)
		clearargs(coms->comv[i]);
	free(coms);
}
