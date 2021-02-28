#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define LINESIZE 1024
#define WORDSIZE 128

enum token { Num, Pipe };

typedef struct {
	char* argv[10];
	int argc;
} SimpleCommand;
SimpleCommand comm = {NULL, 0};

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

	while (*line != '\n') {
		while (*line == ' ')
			line++;
		switch (getword(word, WORDSIZE, line)) {
			case Num:
				comm.argv[comm.argc++] = strdup(word);
				break;
			case Pipe:
				printf("Pipe\n");
				break;
			default:
				return 0;
		}
		while (*line != ' ' && *line != '\n')
			line++;
	}
	comm.argv[comm.argc] = NULL;

	return 1;
}

int getword(char* word, int lim, char* line)
{
	while (--lim && *line != ' ' && *line != '\n') {
		if (*line == '|')
			return Pipe;
		*word++ = *line++;
	}
	*word = '\0';
	return Num;
}

int execute(void)
{
	int ret, status;

	if (comm.argc == 0)
		return 1;

	switch (ret = fork()) {
		case -1:
			return 0;
		case 0:
			execvp(comm.argv[0], comm.argv);
			exit(EXIT_SUCCESS);
		default:
			waitpid(ret, &status, WUNTRACED);
	}

	return 1;
}

int clearcomms(void)
{
	int i;

	for (i=0; i<comm.argc; i++)
		free(comm.argv[i]);
	comm.argc=0;

	return 1;
}
