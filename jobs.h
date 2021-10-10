#ifndef JOBS_H
#define JOBS_H

#include <sys/types.h>

typedef struct _job {
	char *comm;
	pid_t jid;
	pid_t pgid;
	pid_t *pids;
	int npids;
	int state;
	struct _job *next;
} job;

job* jb_create(char *comm, pid_t pgid, int npids);
job* jb_get(pid_t id, int type);

void jb_printone(job *jb);
void jb_printall(void);

void jb_destroy(job *jb);

#endif
