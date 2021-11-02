#ifndef EXEC_H
#define EXEC_H

#include "ast.h"

void execcoms(commands *coms, ioredir* ior);
arguments* expandwc(arguments *args, char *wc);

#endif
