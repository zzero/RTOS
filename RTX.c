
#include "include/RTX.h"

int main(){
	Intialization()
	return 0;
}

void atomic(bool status)
{
	static sigset_t oldmask;
	sigset_t newmask;
	if (on) 
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