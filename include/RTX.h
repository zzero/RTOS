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

/* Process ID */
#define NUMB_PROC 8
#define PROCESSA 1
#define PROCESSB 2
#define PROCESSC 3
#define NULL_PROCESS 4
#define CCI 5
#define CRT_I_PROC 6
#define TIMER_I_PROC 7
#define KB_I_PROC 8

/* Function Headers */
int terminate();
void ProcessA();
void ProcessB();
void ProcessC();
void CCI();

#endif
