#include "ast.h"
#include <stdlib.h>
#include <string.h>

#define DEFMAXARGS 8

struct arguments *creatargs(void)
{
	struct arguments *args = (struct arguments *) malloc(sizeof(struct arguments));
	args->maxarg = DEFMAXARGS;
	args->arg_list = (char **) calloc(args->maxarg, sizeof(char *));
	args->curarg = 0;

	return args;
}

struct arguments *addarg(struct arguments *args, char *arg)
{
	if (args->curarg == args->maxarg) {
		if (!arg)
			args->maxarg++;
		else 
			args->maxarg *= 2;
		args->arg_list = realloc(args->arg_list, args->maxarg*sizeof(char *));
	}
	args->arg_list[args->curarg++] = (!arg) ? NULL : strdup(arg);

	return args;
}

void clearargs(struct arguments *args)
{
	for (int i = 0; i < args->curarg-1; i++)
		free(args->arg_list[i]);
	free(args);
}



#define DEFMAXCOMS 4

struct commands *creatcoms(void)
{
	struct commands *coms = (struct commands *) malloc(sizeof(struct commands));
	coms->maxcom = DEFMAXCOMS;
	coms->com_list = (struct arguments **) calloc(coms->maxcom, sizeof(struct arguments *));
	coms->curcom = 0;

	return coms;
}

struct commands *addcom(struct commands *coms, struct arguments *com)
{
	if (coms->curcom == coms->maxcom) {
		coms->maxcom *= 2;
		coms->com_list = realloc(coms->com_list, coms->maxcom*sizeof(struct arguments *));
	}
	com = addarg(com, NULL);
	coms->com_list[coms->curcom++] = com;

	return coms;
}

void clearcoms(struct commands *coms)
{
	for (int i = 0; i < coms->curcom; i++)
		clearargs(coms->com_list[i]);
	free(coms);
}
