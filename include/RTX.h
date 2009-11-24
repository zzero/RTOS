#ifndef RTX_H
#define RTX_H

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include "kernel.h"
#include "../kernel.c"

/* Function Headers */
int terminate();
void ProcessA();
void ProcessB();
void ProcessC();
void cci();

typedef struct iTableRow
{
	int pid;
	int priority;
	int stacksize;
	int *start_PC;
}iTableRow;

#endif
