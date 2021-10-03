#ifndef EXEC_H
#define EXEC_H

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
#include "ast.h"
#include "sortlist.h"
#include "jobs.h"

void execcoms(struct commands *);
struct arguments *expandwc(struct arguments *, char *);

#endif
