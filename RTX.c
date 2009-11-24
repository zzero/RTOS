#include "include/RTX.h"

int main()
{
	Initialization();
	return 0;
}

int send_console_chars(MsgEnv *msg_env){
	atomic(1);
	int temp;
	temp = K_send_console_chars(msg_env);
	return temp;
	atomic(0);
}

int get_console_chars(MsgEnv *msg_env)
{
	atomic(1);
	int temp;
	temp = K_get_console_chars(msg_env);
	return temp;
	atomic(0);
}

int change_priority(int new_priority, int target_process_id)
{
	atomic(1);
	int temp;
	temp = K_change_priority(new_priority, target_process_id);
	return temp;
	atomic(0);
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

    int i=1;
    for (i=1; i<=NUMB_PROC; i++)
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
{return;}

void ProcessB()
{return;}

void ProcessC()
{return;}

/*Command Console Interface*/
void cci()
{
     /* Output CCI: to display wait for acknowledge, send msg 
     to kb_i_proc to return latest keyboard input*/  
     
     MsgEnv* CCI_env;    
     strcpy(CCI_env->text_area,"CCI: ");
	 send_console_chars(CCI_env);
	 
	
     while (CCI_env->type != DISPLAY_ACKNOWLEDGEMENT)
	 {  
           CCI_env =  K_receive_message();
     }
	 
     get_console_chars(CCI_env);
     
     while (CCI_env->type != CONSOLE_INPUT)
     {
	       CCI_env = K_receive_message();
     }
    
     /*Store text of CCI_env into character string, convert to lowercase*/
     char CCI_store[10]={'\0'};
     strcpy(CCI_env->text_area,CCI_store);
     //stolower(CCI_store);
     
     /*send message envelope to User Process A*/
     if(CCI_store == "s")
     {
        strcpy(CCI_env->text_area,CCI_store); 
        send_message(1, CCI_env); // 1 -> refers to USERPROC_A
        
        send_console_chars(CCI_env);
        while (CCI_env->type != DISPLAY_ACKNOWLEDGEMENT)
	     {  
           CCI_env =  K_receive_message();
         }
       
     }
     
     /*Display Process Status of all processes*/
     else if(CCI_store == "ps")
     {
        strcpy(CCI_env->text_area,CCI_store); 
        request_process_status(CCI_env);
        
        send_console_chars(CCI_env);
        while (CCI_env->type != DISPLAY_ACKNOWLEDGEMENT)
	     {  
           CCI_env =  K_receive_message();
         }      
      
     }
     
     /*Sets CCI wall clock to desired format*/
     else if(CCI_store == "c hh:mm:ss")
     {
         strcpy(CCI_env->text_area,CCI_store);
         
         send_console_chars(CCI_env);
         while (CCI_env->type != DISPLAY_ACKNOWLEDGEMENT)
	     {  
           CCI_env =  K_receive_message();
         } 
             
         MsgEnv* CCI_clock;    
         CCI_clock->type = SET_CLOCK;  //declare SET_CLOCK as global
         strcpy(CCI_env->text_area,CCI_store);
         send_message(CLOCK_PID, CCI_clock); //define CLOCK_PID as global
         
     }
     
     /*Display wall clock*/
     else if(CCI_store =="cd")
     {
         MsgEnv* CCI_clock;    
         CCI_clock->type = CLOCK_ON;  //declare SET_CLOCK as global
         strcpy(CCI_env->text_area,CCI_store);
         send_message(CLOCK_PID, CCI_clock); //define CLOCK_PID as global
         
     }
     
     /*Clear wall clock*/
     else if(CCI_store == "ct")
     {
         strcpy(CCI_env->text_area," ");
         
        send_console_chars(CCI_env);
        while (CCI_env->type != DISPLAY_ACKNOWLEDGEMENT)
	     {  
           CCI_env =  K_receive_message();
         } 
        
     }
     
     /*Display contents of send and recieve trace buffers*/
     else if(CCI_store == "b")
     {
         strcpy(CCI_env->text_area,CCI_store);
         
         get_trace_buffers(CCI_env);
         
         send_console_chars(CCI_env);
         while (CCI_env->type != DISPLAY_ACKNOWLEDGEMENT)
	     {  
           CCI_env =  K_receive_message();
         } 
         
     }
     
     /*Terminate RTX*/
     else if(CCI_store == "t")
     {
         strcpy(CCI_env->text_area,CCI_store);
         
         send_console_chars(CCI_env);
         while (CCI_env->type != DISPLAY_ACKNOWLEDGEMENT)
	     {  
           CCI_env =  K_receive_message();
         } 
         
         sig_handler(SIGINT);
                     
     }
     
     /*Change priority of a specified process*/
     else if( strcmp(CCI_store[0], "n") == 0)
     {
         strcpy(CCI_env->text_area,CCI_store);
         int priority, id; 
         int i;
         for(i = 0; i<10; i++)
         {
            if(i == 1 && strcmp(CCI_store[0], " ") == 0 )
                id = CCI_store[i+1];    
            
            else if(i == 3 && CCI_store[i] == " ")
                 priority = CCI_store[i+1];
             
         }      
         
         change_priority(priority, id);
         
         send_console_chars(CCI_env);
         while (CCI_env->type != DISPLAY_ACKNOWLEDGEMENT)
	     {  
           CCI_env =  K_receive_message();
         } 
     
     }
     
     /*User enters blank space*/
     else if(CCI_store == " ")
     {
         strcpy(CCI_env->text_area,CCI_store);
         
         send_console_chars(CCI_env);
         while (CCI_env->type != DISPLAY_ACKNOWLEDGEMENT)
	     {  
           CCI_env =  K_receive_message();
         }
         
      } 
          
}

void sig_handler(int sig_name)
{
	/*Disable Signals, save pointer to currently active PCB*/
	atomic(1);
	PCB* save = current_process;  
	
	/*Realize Signal, excecute appropriate action*/
	switch(sig_name){
	
	/*Terminate Signal*/
	case SIGINT:
	    terminate();
		break;

	
	/*Timing Signal - service delay requests, complement UALRM*/
	case SIGALRM:  //Alarm Signal (timing)
		  
          current_process ->pid = TIMER_I_PROC;
		  current_process->status = EXECUTING;
          Timer_I_Proc();
		  current_process->status = IDLE;
          break;

    
	/*CRT Signal*/
	case SIGUSR1:  				
          
          current_process ->pid = CRT_I_PROC;
		  current_process->status = EXECUTING;
          CRT_I_Proc();
		  current_process->status = IDLE;
		  break; 

	/*KB Signal*/
	case SIGUSR2:  			
          
          current_process ->pid = KB_I_PROC;
		  current_process->status = EXECUTING;
          KB_I_Proc();
          current_process->status = IDLE;
		  break; 

	default:	
        printf("Unknown Signal!\n");
		terminate();
		exit(1);
		break;

    }
    current_process = save;
    atomic(0);
}
