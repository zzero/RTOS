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
//#include "../kernel.c"

#define STACKSIZE 16384 //whats this
#define SK_OFFSET 16

int cciHandleCRT;
int cciHandleKB;


/* Function Headers */
int terminate();
void ProcessA();
void ProcessB();
void ProcessC();
void cci();
void atomic(int status);
void sig_handler(int sig_name);

typedef struct iTableRow
{
	int pid;
	int priority;
	int stacksize;
	void (*start_PC)(); //Correct me if I am wrong, but start_PC is the ptr to a starting function. It shouldn't be int.(BK)
}iTableRow;

#endif
