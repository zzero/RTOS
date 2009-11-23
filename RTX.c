
#include "include/RTX.h"

int main(){

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

    
int terminate(){
    atomic(1)
    
    for (int i=1; i<=NUMB_PROC; i++)
        free(PCB_finder(i));
   
    MsgEnv *temp;  
    for(temp = ptr_free_envQ; temp!=NULL; temp=temp->next)
         free(temp);
  
    //kill shared memory        
     
    atomic(0)
    printf("SIGNAL RECEIVED..TERMINATING RTX");
    
    }
    
void ProcessA()
{}

void ProcessB()
{}

void ProcessC()
{}
