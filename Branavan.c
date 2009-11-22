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
void KB_I_Proc()
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
void CRT_I_Proc()
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


void Timer_I_Proc()
{
     MsgEnv* msg;   
     msg = K_recieve(); 
     
     while(msg != NULL)
     {
        
        sortMsg(msg);
                      
        msg = K_recieve();
     }
    
    if(tail->tmout_msg.tick_count != 0)
    {
        tail->tmout_msg.tick_count = tail->tmout_msg.tick_count - 1;
        time_list *temp = head; 
        while(temp->next != NULL)
        {
           temp->tmout_msg.tick_count -= temp->next->tmout_msg.tick_count; 
        } 
    }
   
   //verify this process     
    if(tail->tmout_msg.tick_count == 0)
    {
        MsgEnv *send;               
        int temp = tail.source_ID;
        send.source_ID = tail.timeout_process_id;
        send.msg_type = WAKE_UP;
        tail->previous->next = NULL;
        K_send_message(temp, send);
        
    }
    
}


struct timeoutQ()
{
MsgEnv *tmout_msg
struct timeoutQ *next;
}//time_list

typedef struct timeoutQ time_list;



void sortMsg(MsgEng *msg)
{
        //initializing list to be done outside!!!
        time_list *head = NULL;
        time_list *tail = NULL;
        
        //creating new node
        time_list *add;
        if ((add = malloc(sizeof(time_list))) == NULL) {abort();} 
        
        //populate new element fields
        add->toumt_msg = msg;
        
        //adding to an empty list
        if (head == NULL)
        {
           head = add; 
           tail = add;
        }
        
        //adding to end of list
        if(msg.tick_count < tail->tmout_msg.tick_count)
        {
            time_list *temp = head; 
            while(temp != NULL){
                 temp = temp.next;                  
            } 
            temp.next = add;
            add = NULL;
           
            temp = head; 
            
            while(temp->next != NULL)
            {
              temp->tmout_msg.tick_count -= temp->next->tmout_msg.tick_count; 
            } 
                     
        }              
        
        //adding to middle of list
        if(head != NULL && tail != NULL)
        {
            time_list *temp = head;
                
            while((temp->next != NULL) && (msg.tick_count < temp->tmout_msg.tick_count))
            {
                 temp = temp->next;                    
            }
           
            add -> next = temp->next; 
            temp -> next = add;
            
            temp = head; 
            while(temp->next != NULL)
            {
              temp->tmout_msg.tick_count -= temp->next->tmout_msg.tick_count; 
            } 
              
        }
        
        
        
}
//Clock Process
      
void clock_proc()
{
     char clock[CLOCK]={};
     MsgEnv* msgsend;
     msgsend.msg_type = TIMER_REQUEST;
     
       
       
          
}
         
                
     
