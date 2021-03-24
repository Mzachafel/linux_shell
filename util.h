#ifndef MY_UTIL_H
#define MY_UTIL_H

void printprompt(void);
struct arguments *expandwc(struct arguments *, char *, char *);
void execcoms(struct commands *);
void clearvars(void);

#endif
