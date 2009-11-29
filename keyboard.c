/* KB */
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/mman.h>			//mmap()
#include "include/kernel.h"		//our global parameters

void die()	//kills itself
{
	printf("\nKeyboard is aborting\n");
	exit(0);
}

int main(int argc, char * argv[])	//get rtx_pid, fid from RTX during fork
{
	printf("\nKB start\n");
	kb_filename = "kb_sm_file";
	//variables
	caddr_t kb_mmap_ptr;	//ptr to memory mapped
	kb_sm * kb_sm_ptr;		//ptr to shared memory
	int index = 0;			//data index in kb_sm
	char c;					//char inputted
	int rtx_pid, kb_sm_fid;	//get thru arguments
	
	sigset(SIGINT, die);	//register sig to run die
	
	sscanf(argv[1], "%d", &rtx_pid );	//convert from char to int
	sscanf(argv[2], "%d", &kb_sm_fid );	//convert from char to int
	
	//memory map
	kb_mmap_ptr = mmap
	(
		(caddr_t) 0,				//memory location; 0 lets OS choose
		SMSIZE,						//bytes to map
		PROT_READ | PROT_WRITE,		//read and write permissions
		MAP_SHARED,					//accessible by another process
		kb_sm_fid,					//which file is associated with mmap
		(off_t) 0					//offset in page frame
	);					

	if (kb_mmap_ptr == MAP_FAILED)		//if memory map fail, abort KB
	{
		printf("Child memory map has failed, KB is aborting!\n");
		die();
	}

	kb_sm_ptr = (kb_sm *) kb_mmap_ptr;	//kb_sm pointer to the memory mapped
	kb_sm_ptr->status = 0;				//0: ready for KB
										//1: ready for RTX

	sleep(3);
	while(1)	//until killed by parent
	{
	//let rtx initialize, CCI to pop up on screen
		c = getchar();						//wait until input (blocked)

		if ( c != '\n' )					//not carriage return
		{
			if (index < KB_MAXCHAR - 1)		//last one is saved for the null char
			{
				kb_sm_ptr->data[index] = c;	//put input char into kb_sm
				index++;					//increment index
			}
		}

		else	//if carriage return or buffer full
		{
			kb_sm_ptr->data[index] = '\0';	//last char is null
			kb_sm_ptr->status = 1;			//ready for rtx to take away char
			kill(rtx_pid, SIGUSR1);			//send a signal to parent
			index = 0;						//restart index
			while(kb_sm_ptr->status == 1){
				usleep(100000);
			}
			
			//now go back to waiting for user input char
		}
	}
}
