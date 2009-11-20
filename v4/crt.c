/* CRT */
#include <stdlib.h>
#include <stdio.h>			//standard in/out
#include <signal.h>			//signals defined
#include <sys/mman.h>		//memory map
//low
#include "global.h"			//our global parameters

void die()	//kills itself
{
	exit(0);
}

int main(int rtx_pid, int crt_sm_fid)	//get rtx_pid, fid from RTX during fork
{
	sigset(SIGINT, die);//when parent send SIGINT, abort

	int index = 0;	//counter for num of char in crt_sm
	char c;

	//memory map
	mmap_ptr = mmap
	(
		(caddr_t) 0,				//memory location; 0 lets OS choose
		SMSIZE,						//bytes to map
		PROT_READ | PROT_WRITE,		//read and write permissions
		MAP_SHARED,					//accessible by another process
		crt_sm_fid,					//which file is associated with mmap
		(off_t) 0					//offset in page frame	//off_t from types.h
	);	

	if (mmap_ptr == MAP_FAILED)		//if memory map fail, abort KB
	{
		printf("Child memory map has failed, CRT is aborting!\n");
		die();
	}

	crt_sm_ptr = (crt_sm *) mmap_ptr;	//sm pointer to the memory mapped
	crt_sm_ptr->status = 0;				//0: ready for CRT
										//1: ready for RTX

	while(1)	//until killed by parent
	{
		while(kb_sm_ptr->status == 1)
			usleep(SLEEP);

		while(crt_sm_ptr->data[index] != '\0')
		{
			c = data[index];	//get the char to be displayed
			printf ("%c", c);	//print to screen the char
			index+=1;
		}

		printf("\n");	//newline

		crt_sm_ptr->status = 1;		//ready for rtx to put more char
		kill(rtx_pid, SIGUSR2);			//send a signal to parent			
		crt_sm_ptr->index = 0;		//reset index

		//now go back to waiting for status bit to change
	}
}