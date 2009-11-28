#include "include/kernel.h"

PCB *PCB_finder(int pid)
{
	PCB *temp;

	for (temp = ptr_readyQ->p0->head; temp != NULL; temp=temp->next){
		if (temp->pid==pid)
			return temp;
	}
	
	for (temp = ptr_readyQ->p1->head; temp != NULL; temp=temp->next){
		if (temp->pid==pid)
			return temp;
	}
	
	for (temp = ptr_readyQ->p2->head; temp != NULL; temp=temp->next){
		if (temp->pid==pid)
			return temp;
	}
	
	for (temp = ptr_readyQ->p3->head; temp != NULL; temp=temp->next){
		if (temp->pid==pid)
			return temp;
	}
	
	for (temp = ptr_blocked_on_receiveQ->head; temp!=NULL; temp = temp->next){
		if (temp->pid==pid)
			return temp;
	}
	
	for (temp = ptr_blocked_on_requestQ->head; temp!=NULL; temp = temp->next){
		if (temp->pid==pid)
			return temp;
	}
	
	if(pid == crt_i_proc->pid)
		return crt_i_proc;
	
	else if(pid == kb_i_proc->pid)
		return kb_i_proc;
	
	else if(pid == timer_i_proc->pid)
		return timer_i_proc;
	
	
	return NULL;
}

//FIXME: possibly place this in another file?
char* itoa(int numb, char *buffer)
{
	sprintf(buffer, "%d", numb);
	return buffer;
}
	

int K_request_process_status(MsgEnv *msg_env)
{
	PCB *temp;

	char pid[15];
	char status[25];
	char priority[25];
	strcpy(priority, "priority: ");
	strcpy(pid,"PID: ");
	strcpy(status, "status: ");

	int to_return;
	int i;
	char buffer[100];

	for(i=1; i<=(USR_PROC_NUMB); i++){
		temp = PCB_finder(i);
		strcat (pid, itoa(temp->pid, buffer));
		strcat (pid,",");
		strcat (status, itoa(temp->status, buffer));
		strcat (status,",");
		strcat (priority, itoa(temp->priority, buffer));
		strcat (priority,",");
	}
	char proc_status[TEXT_AREA_SIZE];
	strcpy (proc_status, pid);
	strcat (proc_status, "\n");
	strcat (proc_status, status);
	strcat (proc_status, "\n");
	strcat (proc_status, priority);
	strcat (proc_status, "\n");
	strcpy(msg_env->text_area,proc_status);

	to_return = K_send_message(msg_env->dest_id, msg_env); 
	return to_return;	
}
	
int K_change_priority(int new_priority, int target_process_id)
{
	PCB *target_proc = PCB_finder(target_process_id);
	
	if (target_proc == NULL)
		return FAIL;
	else
		target_proc->priority = new_priority;
	
	return SUCCESS;
}

int K_release_msg_env(MsgEnv *msg_env)
{
	if(current_process->status == iPROC)
		return FAIL;
	
	msg_env->sender_id = defaultPID;
	msg_env->dest_id = defaultPID;
	msg_env->type = FREE;
	msg_env->num_clock_ticks = 0;
	strcpy(msg_env->text_area, "");
	enque_msg_to_free_envQ(msg_env);
	
	if(ptr_blocked_on_requestQ->head != NULL)
	{
		PCB *PCB_released;
		PCB_released = deque_PCB_from_blocked_on_requestQ();
		PCB_released->status = READY;
		enque_PCB_to_readyQ(PCB_released);
	}
			
	return SUCCESS;
}

MsgEnv *K_request_msg_env()
{	
	if(current_process->status == iPROC)
		return NULL;
	
	while(ptr_free_envQ ==NULL)
	{
		current_process->status = BLOCKED_ON_REQUEST;
		enque_PCB_to_blocked_on_requestQ(current_process);
		process_switch();
	}
	
	MsgEnv *to_return;
	to_return = deque_msg_from_free_envQ();
	return to_return;
}

int K_send_message(int dest_pid, MsgEnv *msg_env)
{
	PCB *dest_pcb = PCB_finder(dest_pid);
	
	if(dest_pcb == NULL || msg_env == NULL)	
		return FAIL;
	else
	{
		enque_msg_to_PCB(msg_env, dest_pcb);
		
		if(dest_pcb->status == BLOCKED_ON_RECEIVE){
			deque_PCB_from_blocked_on_receiveQ(dest_pcb);
			enque_PCB_to_readyQ(dest_pcb);
		}
	}
	
	//trace_send(msg_env->sender_id, msg_env->dest_id, msg_env->type); TMPLY COMMENTED OUT FOR TESTING
		
	return SUCCESS;
}

MsgEnv *K_receive_message()
{
	printf("current_process status, pid = %d, %d\n",current_process->status,current_process->pid);
	while(current_process->receive_env_head == NULL)
	{
		if(current_process->status == iPROC)
			return NULL;
		else
		{
			current_process->status = BLOCKED_ON_RECEIVE; 	
			enque_PCB_to_blocked_on_receiveQ(current_process);
			process_switch();
		}
	}
	
	MsgEnv *msg_env = deque_msg_from_PCB(current_process);
	//trace_receive(msg_env->sender_id, msg_env->dest_id, msg_env->type);
	
	return msg_env;
}

void trace_send(int sender_id, int dest_id, int type)
{
	trace *new_trace;
	for(new_trace = TBsend->sendTrcBfr_head; new_trace->dest_pid != defaultPID; new_trace=new_trace->next)
	{}

	new_trace->dest_pid = dest_id;
	new_trace->source_pid = sender_id;
	new_trace->message_type = type;
	new_trace->time_stamp = 0; //FIXME: what type?
	
	if(TBsend->trace_numb==16)
	{
		TBsend->sendTrcBfr_head->dest_pid = defaultPID;
		TBsend->sendTrcBfr_head->source_pid = defaultPID;
		TBsend->sendTrcBfr_head->message_type = FREE;
		TBsend->sendTrcBfr_head->time_stamp = 0;
		TBsend->trace_numb = 1;
	}
	else{
		new_trace->next->dest_pid = defaultPID;
		new_trace->next->source_pid = defaultPID;
		new_trace->next->message_type = FREE;
		new_trace->next->time_stamp = 0;
	}
	
	TBsend->sendTrcBfr_tail = new_trace;
	TBsend->trace_numb++;
}
		
void trace_receive(int sender_id, int dest_id, int type)
{
	trace *new_trace;
	for(new_trace = TBreceive->recvTrcBfr_head; new_trace->dest_pid != defaultPID; new_trace=new_trace->next)
	{}

	new_trace->dest_pid = dest_id;
	new_trace->source_pid = sender_id;
	new_trace->message_type = type;
	new_trace->time_stamp = 0; //FIXME: what type?
	
	if(TBreceive->trace_numb==16)
	{
		TBreceive->recvTrcBfr_head->dest_pid = defaultPID;
		TBreceive->recvTrcBfr_head->source_pid = defaultPID;
		TBreceive->recvTrcBfr_head->message_type = FREE;
		TBreceive->recvTrcBfr_head->time_stamp = 0;
		TBreceive->trace_numb = 1;
	}
	else{
		new_trace->next->dest_pid = defaultPID;
		new_trace->next->source_pid = defaultPID;
		new_trace->next->message_type = FREE;
		new_trace->next->time_stamp = 0;
	}
	
	TBreceive->recvTrcBfr_tail= new_trace;
	TBreceive->trace_numb++;
}

int K_send_console_chars(MsgEnv *msg_env)
{
	msg_env->dest_id = crt_i_proc->pid;
	msg_env->type = DISPLAY_ACKNOWLEDGEMENT;
	printf("sending...\n");
	int return_value = K_send_message(crt_i_proc->pid, msg_env);
	
	return return_value;
}

int K_get_console_chars(MsgEnv *msg_env)
{
	msg_env->dest_id = kb_i_proc->pid;
	msg_env->type = CONSOLE_INPUT;
	
	int return_value = K_send_message(kb_i_proc->pid, msg_env);
	
	return return_value;
}
				
int K_get_trace_buffers(MsgEnv *msg_env)
{
	char sendtrc_buf[500];
	strcpy(sendtrc_buf, "sent: \n");

	char rcvtrc_buf[500];
	strcpy(rcvtrc_buf, "received: \n");
	
	char buf[100];
	
	trace *send_temp;
	trace *receive_temp;
	
	for(send_temp = TBsend->sendTrcBfr_head; send_temp != NULL; send_temp=send_temp->next)
	{
		if(send_temp->source_pid == defaultPID)
			send_temp = send_temp->next;
		strcat(sendtrc_buf, itoa(send_temp->dest_pid, buf));
		strcat(sendtrc_buf,",");
		strcat(sendtrc_buf,  itoa(send_temp->source_pid, buf));
		strcat(sendtrc_buf,",");
		strcat(sendtrc_buf,  itoa(send_temp->message_type, buf));
		strcat(sendtrc_buf,",");
		strcat(sendtrc_buf, itoa(send_temp->time_stamp, buf));
		strcat(sendtrc_buf,",\n");
	}
	
	for(receive_temp = TBreceive->recvTrcBfr_head; receive_temp != NULL; receive_temp = receive_temp->next)
	{
		if(receive_temp->source_pid == defaultPID)
			receive_temp = receive_temp->next;
		strcat(rcvtrc_buf, itoa(receive_temp->dest_pid, buf));
		strcat(rcvtrc_buf,",");
		strcat(rcvtrc_buf, itoa(receive_temp->source_pid, buf));
		strcat(rcvtrc_buf,",");
		strcat(rcvtrc_buf, itoa(receive_temp->message_type, buf));
		strcat(rcvtrc_buf,",");
		strcat(rcvtrc_buf, itoa(receive_temp->time_stamp, buf));
		strcat(rcvtrc_buf,",\n");
	}
	
	char to_return[TEXT_AREA_SIZE];
	strcpy(to_return, sendtrc_buf);
	strcat(to_return, "\n");
	strcat(to_return, rcvtrc_buf);
	
	strcpy(msg_env->text_area,to_return);
	msg_env->type = GET_TRACE_BUF;

	int return_value = K_send_message(msg_env->dest_id, msg_env);
	
	return return_value;
}

int K_release_processor()
{
	if(current_process->status !=iPROC)
		process_switch();	

	else
		return FAIL;
	
	return SUCCESS;
}
	
int K_request_delay(int time_delay, int wakeup_code, MsgEnv *msg_env)
{
	msg_env->type = TIMER_REQUEST;
	msg_env->num_clock_ticks = time_delay;
	
	char buf[100];
	strcpy(msg_env->text_area,itoa(wakeup_code,buf));
	msg_env->dest_id = current_process->pid;
	
	int return_value = K_send_message(current_process->pid, msg_env);
	
	return return_value;
}

void process_switch()
{
	PCB *next_pcb;
	PCB *old_pcb;
	
	next_pcb = deque_PCB_from_readyQ(); //get ptr to highest priority ready process
	next_pcb->status = EXECUTING;
	//printf("PROC SWITCH after next_pcb\n");
	
	old_pcb = current_process;
	enque_PCB_to_readyQ(old_pcb);
	
	current_process = next_pcb;
	context_switch(&(old_pcb->context), &(next_pcb->context));
}

void context_switch(jmp_buf *previous, jmp_buf *next)
{
	int return_code = setjmp(*previous); //save the context of the previous process
	if (return_code == 0) 
		longjmp(*next, 1); // start the next process from where it left of	
}

void null_process()
{
	while(1)
		K_release_processor();
}
void KB_I_Proc()
{
     MsgEnv * msgsend = K_receive_message();          //allocate msg env to send to cci
     char kb_txt[KB_MAXCHAR];  
     int index = 0;
     
     while(kb_sm_ptr->data[index] != '\0')
     {
         kb_txt[index] = kb_sm_ptr->data[index];
         index++;
     }
     kb_txt[index] = '\0';
     
     strcpy(msgsend->text_area, kb_txt);
     K_send_message(CCI, msgsend); //difine CCI_pid as global   
     kb_sm_ptr->status = 0;    
}


//CRT I Process   ----   Kernel Function
void CRT_I_Proc()
{
     MsgEnv* msgrecieved;    
     msgrecieved = K_receive_message(); 

     //~ while(msgrecieved == NULL){
		//~ msgrecieved = K_receive_message(); 
		//~ printf("msgreceving\n");
		     //~ }
     //~ printf("msg type %d\n",msgrecieved->sender_id);
     
     //~ strcpy(crt_sm_ptr->data, msgrecieved->text_area); 
     //~ crt_sm_ptr->status = 0; //,M
     if(msgrecieved != NULL){
	     K_send_message(msgrecieved->sender_id, msgrecieved);
	     printf("CRT I PROC DONE..\n");
	     
	     strcpy(crt_sm_ptr->data, msgrecieved->text_area); 
	     crt_sm_ptr->status = 0; 
     }
}


void sortMsg(MsgEnv *msg)
{
        MsgEnv *append = msg;
        
        //adding to an empty list
        if (TimeoutQ->head == NULL)
        {
           TimeoutQ->head = append; 
           TimeoutQ->tail = append;
        }
        
        //adding to end of list
        if( append->num_clock_ticks < TimeoutQ->tail->num_clock_ticks )
        {
            MsgEnv *temp = TimeoutQ->head; 
            while(temp != NULL){
                 temp = temp->next;                  
            } 
            temp->next = append;
            append = NULL;
           
            temp = TimeoutQ->head; 
            
            while(temp->next != NULL)
            {
              temp->num_clock_ticks -= temp->next->num_clock_ticks; 
            } 
                     
        }              
        
        //adding to middle of list
        if(TimeoutQ->head != NULL && TimeoutQ->tail != NULL)
        {
            MsgEnv *temp = TimeoutQ->head;
                
            while( (temp->next != NULL) && (append->num_clock_ticks < temp->num_clock_ticks) )
            {
                 temp = temp->next;                    
            }
           
            append -> next = temp->next; 
            temp -> next = append;
            
            temp = TimeoutQ->head; 
            while(temp->next != NULL)
            {
              temp->num_clock_ticks -= temp->next->num_clock_ticks; 
            } 
              
        }
        
        
        
}

//Timer I Proess   ----   Kernel Function


void Timer_I_Proc()
{
     atomic(1);
     MsgEnv* msg;   
     msg = K_receive_message(); 
     while(msg != NULL)
     {
        
        sortMsg(msg);
                      
        msg = K_receive_message();
     }
    
    if(TimeoutQ->tail->num_clock_ticks != 0)
    {
        TimeoutQ->tail->num_clock_ticks = TimeoutQ->tail->num_clock_ticks - 1;
        MsgEnv *temp = TimeoutQ->head; 
        while(temp->next != NULL)
        {
           temp->num_clock_ticks -= temp->next->num_clock_ticks; 
        } 
    }
   
   //verify this process     
    if(TimeoutQ->tail->num_clock_ticks == 0)
    {
        MsgEnv *send;
		send = (MsgEnv*)malloc(sizeof(MsgEnv));
        int temp = TimeoutQ->tail->sender_id;
        send->sender_id = TimeoutQ->tail->sender_id;
        send->type = WAKE_UP;
        TimeoutQ->tail->next = NULL;
        K_send_message(temp, send);
        
    }
    atomic(0);
}


//Clock Process    ----   Kernel Function
      
void clock_proc()
{
     int clock_time=0;
     char buffer[CLOCK];
     char clock[CLOCK]={};
     MsgEnv* msgsend;
	 msgsend = (MsgEnv*)malloc(sizeof(MsgEnv));
     msgsend->type = TIMER_REQUEST;
     
     MsgEnv* msgdisp;
     msgdisp = (MsgEnv*)malloc(sizeof(MsgEnv));
	 msgdisp->type = DISPLAY_REQUEST;
	 
     
     K_request_delay(100, WAKE_UP, msgsend);
     
     while(msgsend->text_area != itoa(WAKE_UP,buffer)){
         msgsend = K_receive_message();
     }
     
     msgsend = K_receive_message();  // maybe not needed due to previous line
     
     while(msgsend != NULL)
     {
         //dequeue msg
         
         if(msgsend->type == CLOCK_ON)  //define CLOCK_ON as global double check automated increment
         {
             clock_time++;
             K_request_delay(100, WAKE_UP, msgsend);
             
             while(msgsend->text_area != itoa(WAKE_UP,buffer)){
                  msgsend = K_receive_message();
              }
              
              
             clock_time++;
             K_request_delay(100, WAKE_UP, msgsend);
             while(msgsend->text_area != itoa(WAKE_UP,buffer)){
                  msgsend = K_receive_message();
              }
            itoa(clock_time, clock);

         }   
         
         else if(msgsend->type == SET_CLOCK) ///add to header files
         {
              strcpy(clock , "00:00:00");
              //deallocate msg
              
         }   
         
         msgsend = K_receive_message();        
     }
       
     strcpy(msgdisp->text_area, clock);
     int check;
     check = K_send_console_chars(msgdisp);
     //verify check values
       
          
}

/*Queues*/
void enque_msg_to_PCB(MsgEnv *msg_env, PCB* dest)
{
	if(dest->receive_env_head == NULL){
		dest->receive_env_head = msg_env;
		dest->receive_env_tail = msg_env;
	}
	else{
		dest->receive_env_tail->next = msg_env;
		dest->receive_env_tail = msg_env;
	}
	
	msg_env->next = NULL;
}

MsgEnv *deque_msg_from_PCB(PCB* dest)
{
	MsgEnv *to_return = dest->receive_env_head;
	
	if (dest->receive_env_head != NULL){
		dest->receive_env_head = dest->receive_env_head->next;

		if(dest->receive_env_head == NULL)
			dest->receive_env_tail = NULL;

		to_return->next=NULL;
	}
	
	return to_return;
}

void enque_msg_to_free_envQ(MsgEnv *msg_env)
{
	if(msg_env != NULL){
		msg_env->next = ptr_free_envQ;
		ptr_free_envQ = msg_env;
	}//take care of the other case
	
}

MsgEnv* deque_msg_from_free_envQ()
{
	MsgEnv *to_return = ptr_free_envQ;
	ptr_free_envQ = ptr_free_envQ->next;

	to_return->next=NULL;
	
	return to_return;
}
	
void enque_PCB_to_readyQ(PCB *to_enque)
{
	int priority = to_enque->priority;
	to_enque->next = NULL;
	to_enque->status = READY;
	
	if (priority == 0){
		if(ptr_readyQ->p0->head == NULL){
			ptr_readyQ->p0->head = to_enque;
			ptr_readyQ->p0->tail = to_enque;
		}
		else{
			ptr_readyQ->p0->tail->next = to_enque;
			ptr_readyQ->p0->tail = to_enque;
		}
	}
	
	else if (priority == 1){
		if(ptr_readyQ->p1->head == NULL){
			ptr_readyQ->p1->head = to_enque;
			ptr_readyQ->p1->tail = to_enque;
		}
		else{
			ptr_readyQ->p1->tail->next = to_enque;
			ptr_readyQ->p1->tail = to_enque;
		}
	}
	
	else if (priority == 2){
		if(ptr_readyQ->p2->head == NULL){
			ptr_readyQ->p2->head = to_enque;
			ptr_readyQ->p2->tail = to_enque;
		}
		else{
			ptr_readyQ->p2->tail->next = to_enque;
			ptr_readyQ->p2->tail = to_enque;
		}
	}
	
	else if (priority == 3){
		if(ptr_readyQ->p3->head == NULL){
			ptr_readyQ->p3->head = to_enque;
			ptr_readyQ->p3->tail = to_enque;
		}
		else{
			ptr_readyQ->p3->tail->next = to_enque;
			ptr_readyQ->p3->tail = to_enque;
		}
	}
}

PCB *deque_PCB_from_readyQ()
{
	PCB *to_return;
	
	if(ptr_readyQ->p0->head !=NULL){
		to_return = ptr_readyQ->p0->head;
		ptr_readyQ->p0->head = ptr_readyQ->p0->head->next;
		
		if(ptr_readyQ->p0->head==NULL)
			ptr_readyQ->p0->tail = NULL;
		to_return->next = NULL;
	}
	
	else if(ptr_readyQ->p1->head !=NULL){
		to_return = ptr_readyQ->p1->head;
		ptr_readyQ->p1->head = ptr_readyQ->p1->head->next;
		
		if(ptr_readyQ->p1->head==NULL)
			ptr_readyQ->p1->tail = NULL;
		to_return->next = NULL;
	}
	
	else if(ptr_readyQ->p2->head !=NULL){
		to_return = ptr_readyQ->p2->head;
		ptr_readyQ->p2->head = ptr_readyQ->p2->head->next;
		
		if(ptr_readyQ->p2->head==NULL)
			ptr_readyQ->p2->tail = NULL;
		to_return->next = NULL;
	}
	
	else if(ptr_readyQ->p3->head !=NULL){
		to_return = ptr_readyQ->p3->head;
		ptr_readyQ->p3->head = ptr_readyQ->p3->head->next;
		
		if(ptr_readyQ->p3->head==NULL)
			ptr_readyQ->p3->tail = NULL;
		to_return->next = NULL;
	}

	return to_return;
}

void enque_PCB_to_blocked_on_requestQ(PCB *to_enque)
{
	to_enque->status = BLOCKED_ON_REQUEST;
	if (ptr_blocked_on_requestQ->head == NULL){
		ptr_blocked_on_requestQ->head = to_enque;
		ptr_blocked_on_requestQ->tail = to_enque;
	}
	else
		ptr_blocked_on_requestQ->tail->next = to_enque;

	to_enque->next = NULL;
	ptr_blocked_on_requestQ->tail = to_enque;
}

PCB *deque_PCB_from_blocked_on_requestQ()
{
	PCB* to_return = ptr_blocked_on_requestQ->head;

	if(ptr_blocked_on_requestQ->head != NULL){
		ptr_blocked_on_requestQ->head = ptr_blocked_on_requestQ->head->next;

		if(ptr_blocked_on_requestQ->head == NULL)
			ptr_blocked_on_requestQ->tail = NULL;
	}
	
	to_return->next = NULL;

	return to_return;
}

void enque_PCB_to_blocked_on_receiveQ(PCB *to_enque)
{
	to_enque->status = BLOCKED_ON_RECEIVE;
	if (ptr_blocked_on_receiveQ->head == NULL){
		ptr_blocked_on_receiveQ->head = to_enque;
	}
	else{
		to_enque->next = ptr_blocked_on_receiveQ->head;
		ptr_blocked_on_receiveQ->head = to_enque;
	}
}

PCB *deque_PCB_from_blocked_on_receiveQ(PCB *to_deque)
{
	PCB* to_return;
	PCB* prev;

	if(ptr_blocked_on_receiveQ == NULL)
		return NULL;
	
	else{
		prev = ptr_blocked_on_receiveQ->head;

		if(prev == to_deque){
			ptr_blocked_on_receiveQ->head = ptr_blocked_on_receiveQ->head->next;
			prev->next = NULL;
			return prev;
		}
			
		for (to_return = ptr_blocked_on_receiveQ->head->next;  to_return != to_deque;
			to_return = to_return->next)
			{prev = prev->next;}
		
		if(to_return != to_deque)
			return NULL;

		else{
			if(prev->next != NULL)
				prev->next = prev->next->next;
			return to_return;
			to_return->next = NULL;
		}
	}
	

}
