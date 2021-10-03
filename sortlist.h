#ifndef SORTLIST_H
#define SORTLIST_H

#include <stdlib.h>
#include <string.h>
#include "ast.h"

void creatsl(void);
void addtosl(char *arg);
struct arguments *writesl(struct arguments *args);
void clearsl(void);

#endif
