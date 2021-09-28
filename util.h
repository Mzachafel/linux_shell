#ifndef MY_UTIL_H
#define MY_UTIL_H

struct arguments;
struct commands;

struct arguments *expandwc(struct arguments *, char *);
void execcoms(struct commands *);
void clearvars(void);

#endif
