#include "include/RTX.h"

int main()
{
	Initialization();
	return 0;
}

void atomic(int status)
{
	static sigset_t oldmask;
	sigset_t newmask;
	if (status==1) 
	{
		current_process->atomic_count = current_process->atomic_count + 1;
		if (current_process->atomic_count == 1) 
		{
			sigemptyset(&newmask);
			sigaddset(&newmask, 14); //the alarm signal
			sigaddset(&newmask, 2); // the CNTRL-C
			sigaddset(&newmask, SIGUSR1);
			sigaddset(&newmask, SIGUSR2);
			sigprocmask(SIG_BLOCK, &newmask, &oldmask);
		}
	} 
	else 
	{
		current_process->atomic_count = current_process->atomic_count - 1;
		if (current_process->atomic_count == 0) 
		{
			//unblock the signals
			sigprocmask(SIG_SETMASK, &oldmask, NULL);
		}
	}
}

int terminate()
{
    atomic(1);
    
	//free processes
    for (int i=1; i<=NUMB_PROC; i++)
        free(PCB_finder(i));
   
    //free msg env
    MsgEnv *temp;  
    for(temp = ptr_free_envQ; temp!=NULL; temp=temp->next)
         free(temp);
  
    // terminate child process
	printf("\nKill child processes\n");
	kill(kb_pid, SIGINT);
	kill(crt_pid, SIGINT);
	
	//clean up kb_sm
	printf("\nClean up keyboard shared memory\n");
	status = munmap(kb_mmap_ptr, SMSIZE);
	if (status == -1)
		printf("Bad munmap during cleanup\n");
	status = close(kb_sm_fid); 
	if (status == -1)
		printf("Bad close of temporary mmap file during cleanup\n");		
	status = unlink(kb_filename); 
	if (status == -1)
		printf("Bad unlink during claeanup.\n");
	
	//clean up crt_sm
	printf("\nClean up crt shared memory\n");
	status = munmap(crt_mmap_ptr, SMSIZE);
	if (status == -1)
		printf("Bad munmap during cleanup\n");
	status = close(crt_sm_fid); 
	if (status == -1)
		printf("Bad close of temporary mmap file during cleanup\n");		
	status = unlink(crt_filename); 
	if (status == -1)
		printf("Bad unlink during claeanup.\n");       
    
    atomic(0);
	
    printf("\nSIGNAL RECEIVED..TERMINATING RTX\n");
	exit(0);
}
    
void ProcessA()
{}

void ProcessB()
{}

void ProcessC()
{}
