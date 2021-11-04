#include <stdlib.h>
#include <stdio.h>
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
	args->argv[args->curarg++] = (arg) ? strdup(arg) : NULL;
	if (arg) free(arg);
	return args;
}

void clearargs(arguments *args)
{
	int i;
	for (i = 0; i < args->curarg-1; i++)
		free(args->argv[i]);
	free(args->argv);
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
	int i;
	for (i = 0; i < coms->curcom; i++)
		clearargs(coms->comv[i]);
	free(coms->comv);
	free(coms);
}



ioredir* creatior(void)
{
	ioredir *ior = malloc(sizeof(ioredir));
	memset(ior, 0, sizeof(ioredir));
	return ior;
}

ioredir* addior(ioredir* ior, int fd, int append, char* filename)
{
	if (filename != NULL) {
		ior->iorv[fd] = filename;
		ior->append[fd] = append;
	} else {
		ior->iorv[2] = ior->iorv[1];
		ior->append[2] = 1;
	}
	return ior;
}

void clearior(ioredir* ior)
{
	if (ior->iorv[0]) free(ior->iorv[0]);
	if (ior->iorv[1] != ior->iorv[2]) {
		if (ior->iorv[1]) free(ior->iorv[1]);
		if (ior->iorv[2]) free(ior->iorv[2]);
	} else {
		if (ior->iorv[1]) free(ior->iorv[1]);
	}
	free(ior);
}



#define DEFMAXCBLX 4

comblocks* creatcblx(void)
{
	comblocks* cblx = malloc(sizeof(comblocks));
	cblx->cblv = malloc(DEFMAXCBLX * sizeof(struct comblock));
	cblx->maxcbl = DEFMAXCBLX;
	cblx->curcbl = 0;
	return cblx;
}

comblocks* addcbl(comblocks* cblx, commands* coms, ioredir* ior)
{
	if (cblx->curcbl == cblx->maxcbl) {
		cblx->maxcbl *= 2;
		cblx->cblv = realloc(cblx->cblv, cblx->maxcbl * sizeof(struct comblock));
	}
	cblx->cblv[cblx->curcbl].coms = coms;
	cblx->cblv[cblx->curcbl].ior = ior;
	cblx->cblv[cblx->curcbl].place = 0;
	cblx->curcbl++;
	return cblx;
}

comblocks* setplace(comblocks* cblx, int place)
{
	cblx->cblv[cblx->curcbl-1].place = place;
	return cblx;
}

void clearcblx(comblocks *cblx)
{
	int i;
	for (i=0; i<cblx->curcbl; i++) {
		clearcoms(cblx->cblv[i].coms);
		clearior(cblx->cblv[i].ior);
	}
	free(cblx->cblv);
	free(cblx);
}
