#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <regex.h>
#include <dirent.h>
#include <signal.h>
#include <termios.h>
#include "sortlist.h"
#include "jobs.h"
#include "exec.h"

extern char *cmdline;

static void waitfg(job *jb);
static int builtin(commands *coms);

static sigset_t mask_def, mask_ttou;

void sigchldhandler(int sig)
{
	pid_t pid;
	job *jb;
	while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
		jb = jb_get(pid, 0);
		jb->state = 2;
	}
}

static void execcoms(commands *coms, ioredir* ior, int background, char* comm)
{
	if (coms->curcom == 0)
		return;

	int tmpin;
	int tmpout;
	int tmperr;

	int fdin;
	int fdout;
	int fdoutend;
	int fderr;

	tmpin = dup(0);
	if (ior->iorv[0]) {
		if ((fdin = open(ior->iorv[0], O_RDONLY)) == -1)
			{ perror("mzsh: infile"); close(tmpin); return; }
	} else {
		fdin = dup(tmpin);
	} 

	tmpout = dup(1);
	if (ior->iorv[1]) {
		if (ior->append[1]) {
			if ((fdoutend = open(ior->iorv[1], O_WRONLY | O_APPEND | O_CREAT, 0644)) == -1)
				{ perror("mzsh: outfile"); close(tmpin); close(tmpout); close(fdin); return; }
		} else {
			if ((fdoutend = creat(ior->iorv[1], 0644)) == -1)
				{ perror("mzsh: outfile"); close(tmpin); close(tmpout); close(fdin); return; }
		}
	} else {
		fdoutend = dup(tmpout);
	}

	tmperr = dup(2);
	if (ior->iorv[2]) {
		if (ior->append[2]) {
			if ((fderr = open(ior->iorv[2], O_WRONLY | O_APPEND | O_CREAT, 0644)) == -1)
				{ perror("mzsh: errfile"); close(tmpin); close(tmpout); close(tmperr);
					                   close(fdin); close(fdoutend); return; }
		} else {
			if ((fderr = creat(ior->iorv[2], 0644)) == -1)
				{ perror("mzsh: errfile"); close(tmpin); close(tmpout); close(tmperr); 
					                   close(fdin); close(fdoutend); return; }
		}
	} else {
		fderr = dup(tmperr);
	}

	dup2(fderr, 2);
	close(fderr);

	pid_t pid, pgid;
	job *jb;
	int bltinflag;

	for (int i=0; i<coms->curcom; i++) {
		dup2(fdin, 0);
		close(fdin);

		if (i == coms->curcom-1) {
			fdout = fdoutend;
		} else {
			int fdpipe[2];
			pipe(fdpipe);
			fdin = fdpipe[0];
			fdout = fdpipe[1];
		}

		dup2(fdout,1);
		close(fdout);

		if ((bltinflag = builtin(coms))) {
			;
		} else if ((pid = fork()) == 0) {
			if (i==0) pgid = getpid();
			setpgid(0, pgid);
			if (execvp(coms->comv[i]->argv[0], coms->comv[i]->argv) == -1) {
				dup2(tmperr, 2);
				fprintf(stderr, "Command '%s' not found\n", coms->comv[i]->argv[0]);
				_exit(EXIT_SUCCESS);
			}
		} else if (pid < 0)
			break;
		else {
			if (i==0) { 
				pgid = pid;
				jb = jb_create(comm, pgid, coms->curcom);
			}
			jb->pids[i] = pid;
			setpgid(pid, pgid);
			if (!background && i == coms->curcom-1) {
				dup2(tmpout,1);
				tcsetpgrp(STDOUT_FILENO, pgid);
			}
		}
	}

	dup2(tmpin,0);
	dup2(tmpout,1);
	dup2(tmperr,2);
	close(tmpin);
	close(tmpout);
	close(tmperr);
	close(fdoutend);

	if (!background) {
		if (!bltinflag)
			waitfg(jb);
	} else
		jb_printone(jb);
}

void execcblx(comblocks* cblx)
{
	int i;
	char *comm;
	for (i=0, comm=strtok(cmdline, ";&\0"); i<cblx->curcbl; i++, comm=strtok(NULL, ";&\0"))
		execcoms(cblx->cblv[i].coms, cblx->cblv[i].ior, cblx->cblv[i].place, comm);
	clearcblx(cblx);
	free(cmdline);
}

static void waitfg(job *jb)
{
	int status;
	waitpid(jb->pids[jb->npids-1], &status, WUNTRACED);
	if (WIFSTOPPED(status)) {
		jb->state = 1;
		printf("\n");
		jb_printone(jb);
	} else {
		jb_destroy(jb);
	}
	sigemptyset(&mask_ttou);
	sigaddset(&mask_ttou, SIGTTOU);
	sigprocmask(SIG_BLOCK, &mask_ttou, &mask_def);
	tcsetpgrp(STDOUT_FILENO, getpid());
	sigprocmask(SIG_SETMASK, &mask_def, NULL);
}


static int builtin(commands *coms)
{
	if (!strcmp(coms->comv[0]->argv[0], "cd")) {
		if (coms->comv[0]->curarg != 3)
			fprintf(stderr, "cd: incorrect number of arguments\n");
		else
			chdir(coms->comv[0]->argv[1]);
		return 1;
	} else if (!strcmp(coms->comv[0]->argv[0], "exit")) {
		exit(EXIT_SUCCESS);
	} else if (!strcmp(coms->comv[0]->argv[0], "jobs")) {
		if (coms->comv[0]->curarg == 2) {
			jb_printall();
		} else {
			job *jb;
			pid_t jid;
			for (int i=1; coms->comv[0]->argv[i] != NULL; i++) {
				jid = atoi(coms->comv[0]->argv[i]+1);
				if ((jb = jb_get(jid, 1)) != NULL)
					jb_printone(jb);
				else
					fprintf(stderr, "jobs: no job with id %d\n", jid);
			}
		}
		return 1;
	} else if (!strcmp(coms->comv[0]->argv[0], "kill")) {
		if (coms->comv[0]->curarg < 3) {
			printf("kill: incorrect number of arguments\n");
		} else {
			job *jb;
			pid_t id;
			int type;
			for (int i=1; coms->comv[0]->argv[i] != NULL; i++) {
				if (coms->comv[0]->argv[i][0] == '%') {
					id = atoi(coms->comv[0]->argv[i]+1);
					type = 1;
				} else {
					id = atoi(coms->comv[0]->argv[i]);
					type = 0;
				}
				if ((jb = jb_get(id, type)) != NULL)
					kill(-jb->pgid, SIGINT);
				else if (type == 0)
					kill(id, SIGINT);
			}
		}
		return 1;
	} else if (!strcmp(coms->comv[0]->argv[0], "fg")) {
		if (coms->comv[0]->curarg != 3) {
			fprintf(stderr, "fd: incorrect number of arguments\n");
		} else {
			job *jb;
			pid_t jid = atoi(coms->comv[0]->argv[1]+1);
			if ((jb = jb_get(jid, 1)) != NULL) {
				jb->state = 0;
				kill(-jb->pgid, SIGCONT);
				tcsetpgrp(STDOUT_FILENO, jb->pgid);
				waitfg(jb);
			}
		}
		return 1;
	} else if (!strcmp(coms->comv[0]->argv[0], "bg")) {
		if (coms->comv[0]->curarg != 3) {
			fprintf(stderr, "bg: incorrect number of arguments\n");
		} else {
			job *jb;
			pid_t jid = atoi(coms->comv[0]->argv[1]+1);
			if ((jb = jb_get(jid, 1)) != NULL) {
				jb->state = 0;
				kill(-jb->pgid, SIGCONT);
			}
		}
		return 1;
	}
	return 0;
}

static void wildcard(char* prefix, char* suffix)
{
	// skip wildcard expansion if no joker symbols in expression
	if (!strcmp(prefix, "") && !strpbrk(suffix, "*?")) {
		addtosl(suffix);
		return;
	}
	// leaf of wildcard expansion
	if (*suffix == '\0') {
		addtosl(prefix);
		return;
	}

	// get next regex in path
	char *reg = (char*) malloc(2*strlen(suffix)+10);
	char *s = suffix;
	char *r = reg;

	if (*s == '/')
		s++;

	*r++ = '^';
	while (*s != '\0' && *s != '/') {
		if (*s == '*') { *r++='.'; *r++='*'; }
		else if (*s == '?') *r++='.';
		else if (*s == '.') { *r++='\\'; *r++='.'; }
		else *r++=*s;
		s++;
	}
	if (*s == '/')
		s++;
	*r++='$'; *r='\0';

	regex_t preg;
	int rc;

	if ((rc = regcomp(&preg, reg, 0)) != 0)
		return;

	// if path begins with '/' -> absolute, else -> current directory as beginning
	DIR *dir;
	if (!strcmp(prefix, "")) {
		if (suffix[0] == '/')
			dir = opendir("/");
		else
			dir = opendir(".");
	} else
		dir = opendir(prefix);
	if (dir == NULL) {
		regfree(&preg);
		free(reg);
		return;
	}

	// expanding wildcard tree
	struct dirent *ent;
	size_t nmatch = 1;
	regmatch_t pmatch[1];
	while ((ent = readdir(dir)) != NULL)
		if (!regexec(&preg, ent->d_name, nmatch, pmatch, 0)) {
			// skip filename with dot in beginnging if wildcard doesn't start with dot
			if (ent->d_name[0] == '.' && suffix[0] != '.')
				continue;
			else {
				char* tmp = (char*) malloc(strlen(prefix)+strlen(ent->d_name)+2);
				strcpy(tmp, prefix); 
				if (strcmp(prefix, "") || suffix[0] == '/') strcat(tmp, "/"); 
				strcat(tmp, ent->d_name);
				wildcard(tmp, s);
				free(tmp);
			}
		}
	regfree(&preg);
	free(reg);
	closedir(dir);
}

arguments* expandwc(arguments *args, char *wc)
{
	creatsl();
	wildcard("", wc);
	args = writesl(args);
	clearsl();
	free(wc);
	return args;
}
