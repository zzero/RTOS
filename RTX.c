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

    
int terminate(){
    atomic(1);
    int i=1;
    for (i=1; i<=NUMB_PROC; i++)
        free(PCB_finder(i));
   
    MsgEnv *temp;  
    for(temp = ptr_free_envQ; temp!=NULL; temp=temp->next)
         free(temp);
  
    //kill shared memory        
     
    atomic(0);
    printf("SIGNAL RECEIVED..TERMINATING RTX");
}
    
void ProcessA()
{}

void ProcessB()
{}

void ProcessC()
{}

/*Command Console Interface*/
void CCI()
{
     /* Output CCI: to display wait for acknowledge, send msg 
     to kb_i_proc to return latest keyboard input*/  
     
     MsgEnv* CCI_env;    
     CCI_env.txt_area = "CCI:";
	 send_console_chars(CCI_env);
	 
	
     while (CCI_env.msg_type != DISPLAY_ACKNOWLEDGEMENT)
	 {  
           CCI_env =  receive_message();
     }
	 
     get_console_chars(CCI_env);
     
     while (CCI_eng.msg_type != CONSOLE_INPUT)
     {
	       CCI_env = receive_message();
     }
    
     /*Store text of CCI_env into character string, convert to lowercase*/
     char CCI_store[10]={'\0'};
     CCI_store = CCI_env.txt_area;
     stolower(CCI_store);
     
     /*send message envelope to User Process A*/
     if(CCI_store == "s")
     {
        CCI_env.txt_area = CCI_store; 
        send_message(1, CCI_env); // 1 -> refers to USERPROC_A
        
        send_console_chars(CCI_env);
        while (CCI_env.msg_type != DISPLAY_ACKNOWLEDGEMENT)
	     {  
           CCI_env =  receive_message();
         }
       
       break;
       
     }
     
     /*Display Process Status of all processes*/
     else if(CCI_store == "ps")
     {
        CCI_env.txt_area = CCI_store; 
        request_process_status(CCI_env);
        
        send_console_chars(CCI_env);
        while (CCI_env.msg_type != DISPLAY_ACKNOWLEDGEMENT)
	     {  
           CCI_env =  receive_message();
         }      
     
        break; 
     }
     
     /*Sets CCI wall clock to desired format*/
     else if(CCI_store == "c hh:mm:ss")
     {
         CCI_env.txt_area = CCI_store;
         
         send_console_chars(CCI_env);
         while (CCI_env.msg_type != DISPLAY_ACKNOWLEDGEMENT)
	     {  
           CCI_env =  receive_message();
         } 
             
         MsgEnv* CCI_clock;    
         CCI_clock.msg_type = SET_CLOCK;  //declare SET_CLOCK as global
         CCI_clock.txt_area = CCI_store;
         send_message(CLOCK_pid, CCI_clock); //define CLOCK_pid as global
         
         break;
      
     }
     
     /*Display wall clock*/
     else if(CCI_store =="cd")
     {
         MsgEnv* CCI_clock;    
         CCI_clock.msg_type = CLOCK_ON;  //declare SET_CLOCK as global
         CCI_clock.txt_area = CCI_store;
         send_message(CLOCK_pid, CCI_clock); //define CLOCK_pid as global
         
         break;
     
     }
     
     /*Clear wall clock*/
     else if(CCI_store == "ct")
     {
         CCI_env.txt_area = " ";
         
        send_console_chars(CCI_env);
        while (CCI_env.msg_type != DISPLAY_ACKNOWLEDGEMENT)
	     {  
           CCI_env =  receive_message();
         } 
        
       break;       
     }
     
     /*Display contents of send and recieve trace buffers*/
     else if(CCI_store == "b")
     {
         CCI_env.txt_area = CCI_store;
         
         get_trace_buffers(CCI_env);
         
         send_console_chars(CCI_env);
         while (CCI_env.msg_type != DISPLAY_ACKNOWLEDGEMENT)
	     {  
           CCI_env =  receive_message();
         } 
         
      break;   
     }
     
     /*Terminate RTX*/
     else if(CCI_store == "t")
     {
         CCI_env.txt_area = CCI_store;
         
         send_console_chars(CCI_env);
         while (CCI_env.msg_type != DISPLAY_ACKNOWLEDGEMENT)
	     {  
           CCI_env =  receive_message();
         } 
         
         sig_handler(SIGINT);
                     
     }
     
     /*Change priority of a specified process*/
     else if(CCI_store[0] == "n")
     {
         CCI_env.txt_area = CCI_store;
         int priority, id; 
         
         for(int i = 0; i<10; i++)
         {
            if(i == 1 && CCI_store[i] == " ")
                id = CCI_store[i+1];    
            
            else if(i == 3 && CCI_store[i] == " ")
                 priority = CCI_store[i+1];
             
         }      
         
         change_priority(priority, id);
         
         send_console_chars(CCI_env);
         while (CCI_env.msg_type != DISPLAY_ACKNOWLEDGEMENT)
	     {  
           CCI_env =  receive_message();
         } 
     
     }
     
     /*User enters blank space*/
     else if(CCI_store == " ")
     {
         CCI_env.txt_area = CCI_store;
         
         send_console_chars(CCI_env);
         while (CCI_env.msg_type != DISPLAY_ACKNOWLEDGEMENT)
	     {  
           CCI_env =  receive_message();
         }
         
         break;
      } 
     
          
}