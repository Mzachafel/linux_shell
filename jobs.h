#ifndef JOBS_H
#define JOBS_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#define getstate(x) (x?"Stopped":"Running")

typedef struct _job {
	char *comm;
	pid_t jid;
	pid_t pgid;
	pid_t *pids;
	int npids;
	int state;
	struct _job *next;
} job;

pid_t jb_create(char *comm, pid_t pgid, int npids);
void jb_addpid(pid_t pid);
void jb_destroy(pid_t pid);
void jb_print(void);
job *jb_getattr(pid_t id, int type);

#endif
