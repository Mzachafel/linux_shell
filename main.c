#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

#define LINESIZE 1024
#define WORDSIZE 128

enum token { Num, Pipe, In, Out };

typedef struct {
	char* argv[10];
	int argc;
} Command;
Command comms[10];
int ncomm;
char* infile;
char* outfile;

int parseline(char*);
int getword(char*, int, char*);
int execute(void);
int clearcomms(void);

int main(int argc, char* argv[])
{
	char line[LINESIZE];

	printf("$ ");
	while (fgets(line, LINESIZE, stdin) != NULL) {
		if (!parseline(line))
			fputs("Error parsing line\n", stderr);
		if (!execute())
			fputs("Error executing command\n", stderr);
		if (!clearcomms())
			fputs("Error freeing memory\n", stderr);
		printf("$ ");
	}
	printf("\n");

	return 0;
}

int parseline(char* line)
{
	char word[WORDSIZE];

	ncomm=0;
	comms[ncomm].argc = 0;

	while (*line != '\n') {
		while (*line == ' ')
			line++;
		switch (getword(word, WORDSIZE, line)) {
			case Num:
				comms[ncomm].argv[comms[ncomm].argc++] = strdup(word);
				break;
			case Pipe:
				comms[ncomm].argv[comms[ncomm].argc] = NULL;
				comms[++ncomm].argc = 0;
				break;
			case In:
				line++;
				while (*line == ' ')
					line++;
				getword(word, WORDSIZE, line);
				infile = strdup(word);
				break;
			case Out:
				line++;
				while (*line == ' ')
					line++;
				getword(word, WORDSIZE, line);
				outfile = strdup(word);
				break;
			default:
				return 0;
		}
		while (*line != ' ' && *line != '\n')
			line++;
	}
	comms[ncomm].argv[comms[ncomm].argc] = NULL;

	return 1;
}

int getword(char* word, int lim, char* line)
{
	if (*line == '|')
		return Pipe;
	if (*line == '<')
		return In;
	if (*line == '>')
		return Out;
	while (--lim && *line != ' ' && *line != '\n') {
		*word++ = *line++;
	}
	*word = '\0';
	return Num;
}

int execute(void)
{
	int ret, status, i;

	if (comms[0].argc == 0)
		return 1;

	int tmpin = dup(0);
	int tmpout = dup(1);

	int fdin;
	int fdout;
	if (infile)
		fdin = open(infile, O_RDONLY);
	else
		fdin = dup(tmpin);

	for (int i=0; i<=ncomm; i++) {
		dup2(fdin, 0);
		close(fdin);

		if (i == ncomm)
			if (outfile)
				fdout = creat(outfile, 0644);
			else
				fdout = dup(tmpout);
		else {
			int fdpipe[2];
			pipe(fdpipe);
			fdin = fdpipe[0];
			fdout = fdpipe[1];
		}

		dup2(fdout,1);
		close(fdout);

		switch (ret = fork()) {
			case -1:
				return 0;
			case 0:
				execvp(comms[i].argv[0], comms[i].argv);
				exit(EXIT_SUCCESS);
		}
	}

	dup2(tmpin,0);
	dup2(tmpout,1);
	close(tmpin);
	close(tmpout);

	waitpid(ret, &status, WUNTRACED);

	return 1;
}

int clearcomms(void)
{
	int i, j;

	for (i=0; i<=ncomm; i++)
		for (j=0; j<comms[i].argc; j++)
			free(comms[i].argv[j]);
	if (infile) {
		free(infile);
		infile = NULL;
	}
	if (outfile) {
		free(outfile);
		outfile = NULL;
	}

	return 1;
}