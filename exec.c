#include "exec.h"

extern char *cmdline;
extern char *infile;
extern char *outfile;
extern int append;
extern int background;

static int builtin(struct commands *coms);
static void clearvars(void);

static sigset_t mask_def, mask_ttou;

void execcoms(struct commands *coms)
{
	if (coms->curcom == 0 || builtin(coms))
		return;

	int tmpin = dup(0);
	int tmpout = dup(1);

	int fdin = (infile) ? open(infile, O_RDONLY) : dup(tmpin);
	int fdout;

	pid_t jid, pid, pgid;

	for (int i=0; i<coms->curcom; i++) {
		dup2(fdin, 0);
		close(fdin);

		if (i == coms->curcom-1) {
			fdout = (outfile) ? (append) ? open(outfile, O_WRONLY | O_APPEND | O_CREAT, 0644)
				                     : creat(outfile, 0644)
					  : dup(tmpout);
		} else {
			int fdpipe[2];
			pipe(fdpipe);
			fdin = fdpipe[0];
			fdout = fdpipe[1];
		}

		dup2(fdout,1);
		close(fdout);

		if ((pid = fork()) == 0) {
			if (i==0) pgid = getpid();
			setpgid(0, pgid);
			if (execvp(coms->com_list[i]->arg_list[0], coms->com_list[i]->arg_list) == -1) {
				dup2(tmpout, 1);
				printf("Command '%s' not found\n", coms->com_list[i]->arg_list[0]);
				exit(EXIT_SUCCESS);
			}
		} else if (pid < 0)
			break;
		else {
			if (i==0) { 
				pgid = pid;
				jid = jb_create(cmdline, pgid);
			}
			setpgid(pid, pgid);
			if (!background && i == coms->curcom-1)
				tcsetpgrp(STDOUT_FILENO, pgid);
		}
	}

	dup2(tmpin,0);
	dup2(tmpout,1);
	close(tmpin);
	close(tmpout);

	if (!background) {
		int status;
		waitpid(pid, &status, WUNTRACED);
		if (WIFSTOPPED(status)) {
			printf("[%d] %s\n", jid, cmdline);
		} else {
			jb_destroy(jid);
		}

		sigemptyset(&mask_ttou);
		sigaddset(&mask_ttou, SIGTTOU);
		sigprocmask(SIG_BLOCK, &mask_ttou, &mask_def);
		tcsetpgrp(STDOUT_FILENO, getpid());
		sigprocmask(SIG_SETMASK, &mask_def, NULL);
	} else
		printf("[%d] %s\n", jid, cmdline);

	clearvars();
}

static int builtin(struct commands *coms)
{
	if (!strcmp(coms->com_list[0]->arg_list[0], "cd"))
		if (coms->com_list[0]->curarg > 2 && coms->com_list[0]->arg_list[2] != NULL) {
			printf("cd: Too many arguments\n");
			return 1;
		} else {
			chdir(coms->com_list[0]->arg_list[1]);
			return 1;
		}
	else if (!strcmp(coms->com_list[0]->arg_list[0], "exit"))
		exit(EXIT_SUCCESS);
	else if (!strcmp(coms->com_list[0]->arg_list[0], "jobs")) {
		jb_print();
		return 1;
	}
	return 0;
}

static void clearvars(void)
{
	free(cmdline);
	if (infile) {
		free(infile);
		infile = NULL;
	}
	if (outfile) {
		free(outfile);
		outfile = NULL;
	}
	append = 0;
	background = 0;
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
		*s++;
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
	closedir(dir);
}

struct arguments *expandwc(struct arguments *args, char *wc)
{
	creatsl();
	wildcard("", wc);
	args = writesl(args);
	clearsl();
	return args;
}
