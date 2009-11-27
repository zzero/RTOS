/* CRT */
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/mman.h>			//mmap()
#include "include/kernel.h"		//our global parameters

void die()	//kills itself
{
	printf("\nCRT is aborting\n");
	exit(0);
}

int main(int argc, char * argv[])	//get rtx_pid, fid from RTX during fork
{
	printf("\nCRT start\n");
	crt_filename = "crt_sm_file";
	//variables
	caddr_t crt_mmap_ptr;	//ptr to memory mapped
	crt_sm * crt_sm_ptr;	//ptr to shared memory
	int index = 0;			//data index in crt_sm
	char c;					//char inputted
	int rtx_pid, crt_sm_fid;//get thru arguments
	
	sigset(SIGINT, die);	//register sig to run die

	sscanf(argv[1], "%d", &rtx_pid );	//convert from char to int
	sscanf(argv[2], "%d", &crt_sm_fid );//convert from char to int
	
	//memory map
	crt_mmap_ptr = mmap
	(
		(caddr_t) 0,				//memory location; 0 lets OS choose
		SMSIZE,						//bytes to map
		PROT_READ | PROT_WRITE,		//read and write permissions
		MAP_SHARED,					//accessible by another process
		crt_sm_fid,					//which file is associated with mmap
		(off_t) 0					//offset in page frame
	);	

	if (crt_mmap_ptr == MAP_FAILED)		//if memory map fail, abort CRT
	{
		printf("Child memory map has failed, CRT is aborting!\n");
		die();
	}

	crt_sm_ptr = (crt_sm *) crt_mmap_ptr;	//crt_sm pointer to the memory mapped
	crt_sm_ptr->status = 0;					//0: ready for CRT
											//1: ready for RTX

	while(1)	//until killed by parent
	{
		while(crt_sm_ptr->status == 1)
			usleep(SLEEP);

		while(crt_sm_ptr->data[index] != '\0')
		{
			c = crt_sm_ptr->data[index];		//get the char to be displayed
			printf ("%c", c);		//print to screen the char
			index+=1;
		}

		printf("\n");	//newline

		crt_sm_ptr->status = 1;		//ready for rtx to put more char
		kill(rtx_pid, SIGUSR2);		//send a signal to parent			
		index = 0;					//reset index

		//now go back to waiting for status bit to change
	}
}
