#ifndef JOBS_H
#define JOBS_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

pid_t jb_create(char *comm, pid_t pgid);
void jb_destroy(pid_t jid);
void jb_print(void);

#endif
