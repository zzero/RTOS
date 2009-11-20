#define CLOCK 8

void sig_handler(int sig_name)
{
	/*Disable Signals, save pointer to currently active PCB*/
	atomic(on);
	pcb* save = RTX.current_process;  
	
	/*Realize Signal, excecute appropriate action*/
	switch(sig_name){
	
	/*Terminate Signal*/
	case SIGINT:
	    terminate();
		break;

	
	/*Timing Signal - service delay requests, complement UALRM*/
	case SIGALRM:  //Alarm Signal (timing)
		  
          RTX.current_process = RTX.pid_to_PCBptr(TIMER_I_Proc);
		  RTX.current_process->status = EXECUTING;
          Timer_I_Proc();
		  RTX.current_process->status = IDLE;
          break;

    
	/*CRT Signal*/
	case SIGUSR1:  				
          
          RTX.current_process = RTX.pid_to_PCBptr(CRT_I_PROC);
		  RTX.current_process->status = EXECUTING;
          CRT_I_Proc();
		  RTX.current_process->status = IDLE;
		  break; 

	/*KB Signal*/
	case SIGURS2:  			
          
          RTX.current_process = RTX.pid_to_PCBptr(KB_I_PROC);
		  RTX.current_process->status = EXECUTING;
          KB_I_Proc();
          RTX.current_process->status = IDLE;
		  break; 

	default:	
        printf("Unknown Signal!\n");
		terminate();
		exit(1);
		break;

    }
    RTX.current_process = save;
    atomic(off);
}


//keyboard I Process
void kb_i_proc()
{
     MsgEnv* msgsend;    
     msgsend=K_recieve();
     kb_sm * kb_sm_ptr;
     
     if(msgsend != NULL)
                char kb_txt[KB_MAXCHAR]={"\0"}; //check
     
     if(kb_sm_ptr->status == 1 && in_mem_p -> indata[0] != '\0')
               for(int i = 0; i<KB_MAXCHAR; i++)
                       kb_txt[i] = kb_sm_ptr->data[i];
                       
     msgsend.msg_txt = kb_txt;
     K_send_message(CCI_pid, msgsend);
     
     kb_sm_ptr->status = 0;
}

//CRT I Process
void crt_i_proc()
{
     MsgEnv* msgsend;    
     msgsend=K_recieve(); 
     crt_sm * crt_sm_ptr;
     
     if(msgsend != NULL)
     {
                char crt_txt[CRT_MAXCHAR]={"\0"}; 
                crt_sm_ptr = msgsend.msg_txt; 
                crt_sm_ptr->status = 1;
                
     }
     
}

//Timer I Proess

void timer_i_proc()
{
     MsgEnv* msg;   
     msg=K_recieve(); 
     
     while(msgsend!=NULL){

        
        tick* temp;
        tick* key;
         
        temp = head;
        
        if(temp != NULL){
                while(temp->next != NULL && msg.tick_count > temp->next->msg.tick_count)
                                 temp = temp->next; 
        }
    
        key->msg.tick_count = msg.tick_count - temp->next.tick_count; //consider case for first item entered
        
        key->next=temp->next;
        temp->next = key; // ensure adding to front of list points to null
        
        msg = K_recieve();
    }
    
    if(time_list != 0)
        tail.tick_count = tail.tick_count - 1;
        
    if(tail.tick_count == 0 )
    {
        MsgEnv *send;               
        int temp = tail.source_ID;
        send.source_ID = tail.timeout_process_id;
        send.msg_type = WAKE_UP;
        tail->previous->next = NULL;
        
        K_send_message(temp, send);
        
    }
    
}

//Clock Process
      
void clock_proc()
{
     char clock[CLOCK]={};
     MsgEnv* msgsend;
     msgsend.msg_type = TIMER_REQUEST;
     
       
       
          
}
         
                
     
