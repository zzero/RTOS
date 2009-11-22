/* KB */
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/mman.h>			//memory map
#include "include/global.h"			//our global parameters
#include "include/RTX.h"				//primitives

//#include <sys/wait.h>		//wait() function. don't think we need this
//#include <fcntl.h>		//open() function. don't think we need this
//#include <sys/types.h>	//might need it for caddr_t struct

void die()	//kills itself
{
	terminate();
}

int main(int rtx_pid, int kb_sm_fid)	//get rtx_pid, fid from RTX during fork
{
	//variables
	caddr_t mmap_ptr;	//ptr to memory mapped	//where is this struc from?
	kb_sm * kb_sm_ptr;	//ptr to shared memory
	int index = 0;		//index of char data in shared memory
	char c;				//char inputted

	sigset(SIGINT,die);	//register SIGINT to run die
	
	//memory map
	mmap_ptr = mmap
	(
		(caddr_t) 0,			//memory location; 0 lets OS choose
		SMSIZE,						//bytes to map
		PROT_READ | PROT_WRITE,		//read and write permissions
		MAP_SHARED,					//accessible by another process
		kb_sm_fid,					//which file is associated with mmap
		(off_t) 0					//offset in page frame	//off_t from types.h
	);					

	if (mmap_ptr == MAP_FAILED)		//if memory map fail, abort KB
	{
		printf("Child memory map has failed, KB is aborting!\n");
		die();
	}

	kb_sm_ptr = (kb_sm *) mmap_ptr;	//sm pointer to the memory mapped
	kb_sm_ptr->status = 0;			//0: ready for KB
									//1: ready for RTX

	while(1)	//until killed by parent
	{
		c = getchar();						//wait until input (blocked)

		if ( c != '\n' )					//not carriage return
		{
			if (index < KB_MAXCHAR - 1)		//last one is saved for the null char
			{
				kb_sm_ptr->data[index] = c;	//put input char into sm
				index+=1;					//increment index
			}
		}

		else	//if carriage return or buffer full
		{
			kb_sm_ptr->data[index] = '\0';	//last char is null
			kb_sm_ptr->status = 1;			//ready for rtx to take away char
			kill(rtx_pid, SIGUSR1);				//send a signal to parent
			index = 0;						//restart index

			while(kb_sm_ptr->status == 1)
				usleep(SLEEP);
			
			//now go back to waiting for user input char
		}
	}
}