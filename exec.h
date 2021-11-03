#ifndef EXEC_H
#define EXEC_H

#include "ast.h"

void execcblx(comblocks* cblx);
arguments* expandwc(arguments *args, char *wc);

#endif
