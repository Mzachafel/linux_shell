#ifndef AST_H
#define AST_H

typedef struct _arguments {
	char **argv;
	int maxarg;
	int curarg;
} arguments;
	
arguments* creatargs(void);
arguments* addarg(arguments* args, char* arg);
void clearargs(arguments* args);

typedef struct _commands {
	arguments **comv;
	int maxcom;
	int curcom;
} commands;

commands* creatcoms(void);
commands* addcom(commands* coms, arguments* com);
void clearcoms(commands* coms);

typedef struct _ioredir {
	char *iorv[3];
	int append[3];
} ioredir;

ioredir* creatior(void);
ioredir* addior(ioredir* ior, int fd, int append, char* filename);
void clearior(ioredir* ior);

struct comblock {
	commands *coms;
	ioredir *ior;
	int place;
};

typedef struct _comblocks {
	struct comblock *cblv;
	int maxcbl;
	int curcbl;
} comblocks;

comblocks* creatcblx(void);
comblocks* addcbl(comblocks* cblx, commands* coms, ioredir* ior);
comblocks* setplace(comblocks* cblx, int place);
void clearcblx(comblocks* cblx);

#endif
