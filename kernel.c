#include "include/kernel.h"

#define STACKSIZE 16384 //whats this
#define SK_OFFSET 16

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
}

//FIXME: possibly place this in another file?
char* itoa(int numb, char *buffer)
{
	sprintf(buffer, "%d", numb);
	return buffer;
}
	

MsgEnv* K_request_process_status(MsgEnv *msg_env)
{
	PCB *temp;
	char *buffer;
	char *pid = "PID: ";
	char *status = "status: ";
	char *priority = "priority: ";
	int num_proc = 0;
	
	for (temp = ptr_blocked_on_requestQ->head; temp!=NULL; temp = temp->next)
	{
		strcat (pid,itoa(temp->pid,buffer));
		strcat (pid,",");
		strcat (status, itoa(temp->status,buffer));
		strcat (status,",");
		strcat (priority, itoa(temp->priority,buffer));
		strcat (priority,",");
	}
		
	char proc_status[TEXT_AREA_SIZE] = "";
	strcat (proc_status,pid);
	strcat (proc_status,"\n");
	strcat (proc_status,status);
	strcat (proc_status,"\n");
	strcat (proc_status,priority);
	strcat (proc_status,"\n");
	
	*(msg_env->text_area) = *(proc_status);
	K_send_message(msg_env->dest_id, msg_env);

	return msg_env;	
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
	strcpy(msg_env->text_area,"");
	
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
		
		if(dest_pcb->status = BLOCKED_ON_RECEIVE)
			enque_PCB_to_readyQ(dest_pcb);
	}
	
	trace_send(msg_env->sender_id, msg_env->dest_id, msg_env->type);
		
	return SUCCESS;
}

MsgEnv *K_receive_message()
{
	while(current_process->receive_env_head == NULL)
	{
		if(current_process->status = iPROC)
			return NULL;
		else
		{
			current_process->status = BLOCKED_ON_RECEIVE; 	
			enque_PCB_to_blocked_on_receiveQ(current_process);
			process_switch();
		}
	}
	
	MsgEnv *msg_env = deque_msg_from_PCB(current_process);
	trace_receive(msg_env->sender_id, msg_env->dest_id, msg_env->type);
	
	return msg_env;
}

void trace_send(int sender_id, int dest_id, int type)
{
	trace *new_trace = (trace*)malloc(sizeof(trace));
	new_trace->dest_pid = dest_id;
	new_trace->source_pid = sender_id;
	new_trace->message_type = type;
	new_trace->time_stamp = 0; //FIXME: what type?
	
	if(TBsend->trace_numb>=16)
	{
		trace *head = TBsend->sendTrcBfr_head;
		TBsend->sendTrcBfr_head = TBsend->sendTrcBfr_head->next;
		free(head);
	}
	
	new_trace->prev = TBsend->sendTrcBfr_tail;
	new_trace->next = NULL;
	TBsend->sendTrcBfr_tail->next = new_trace;
	TBsend->sendTrcBfr_tail = new_trace;
	TBsend->trace_numb++;
}
		
void trace_receive(int sender_id, int dest_id, int type)
{

	trace *new_trace = (trace*)malloc(sizeof(trace));
	new_trace->dest_pid = dest_id;
	new_trace->source_pid = sender_id;
	new_trace->message_type = type;
	new_trace->time_stamp = 0; //FIXME: what type?
	
	if(TBreceive->trace_numb>=16)
	{
		trace *head = TBreceive->recvTrcBfr_head;
		TBreceive->recvTrcBfr_head = TBreceive->recvTrcBfr_head->next;
		free(head);
	}
	
	new_trace->prev = TBreceive->recvTrcBfr_tail;
	new_trace->next = NULL;
	TBreceive->recvTrcBfr_tail->next = new_trace;
	TBreceive->recvTrcBfr_tail = new_trace;
	TBreceive->trace_numb++;
}

int K_send_console_chars(MsgEnv *msg_env)
{
	msg_env->dest_id = crt_i_proc->pid;
	msg_env->type = DISPLAY_ACKNOWLEDGEMENT;
	
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
	char *sendtrc_buf = "sent: \n";
	char *rcvtrc_buf = "received: \n";
	char *buf;
	
	trace *send_temp;
	trace *receive_temp;
	
	for(send_temp = TBsend->sendTrcBfr_head; send_temp != NULL; send_temp=send_temp->next)
	{
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
		strcat(rcvtrc_buf, itoa(receive_temp->dest_pid, buf));
		strcat(rcvtrc_buf,",");
		strcat(rcvtrc_buf, itoa(receive_temp->source_pid, buf));
		strcat(rcvtrc_buf,",");
		strcat(rcvtrc_buf, itoa(receive_temp->message_type, buf));
		strcat(rcvtrc_buf,",");
		strcat(rcvtrc_buf, itoa(receive_temp->time_stamp, buf));
		strcat(rcvtrc_buf,",\n");
	}
	
	char *to_return = "";
	strcat(to_return, sendtrc_buf);
	strcat(to_return, rcvtrc_buf);
	strcpy(msg_env->text_area,to_return);
	
	int return_value = K_send_message(msg_env->dest_id, msg_env);
	
	return return_value;
}

int K_release_processor()
{
	if(current_process->status !=iPROC)
		enque_PCB_to_readyQ(current_process);
	else
		return FAIL;
	
	process_switch();
	
	return SUCCESS;
}
	
int K_request_delay(int time_delay, int wakeup_code, MsgEnv *msg_env)
{
	msg_env->type = TIMER_REQUEST;
	msg_env->num_clock_ticks = time_delay;
	
	char *buf;
	strcpy(msg_env->text_area,itoa(wakeup_code,buf));
	msg_env->dest_id = current_process->pid;
	
	int return_value = K_send_message(current_process->pid, msg_env);
	
	return return_value;
}

/*Queues*/
void enque_msg_to_PCB(MsgEnv *msg_env, PCB* dest)
{
	dest->receive_env_tail->next = msg_env;
	msg_env->next = NULL;
	dest->receive_env_tail = msg_env;
}

MsgEnv *deque_msg_from_PCB(PCB* dest)
{
	MsgEnv *to_return = dest->receive_env_head;
	dest->receive_env_head = dest->receive_env_head->next;
	to_return->next=NULL;
	
	return to_return;
}

void enque_msg_to_free_envQ(MsgEnv *msg_env)
{
	msg_env->next = ptr_free_envQ;
	ptr_free_envQ = msg_env;
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
		ptr_readyQ->p0->tail->next = to_enque;
		ptr_readyQ->p0->tail = to_enque;
	}
	else if (priority == 1){
		ptr_readyQ->p1->tail->next = to_enque;
		ptr_readyQ->p1->tail = to_enque;
	}
	else if (priority == 2){
		ptr_readyQ->p2->tail->next = to_enque;
		ptr_readyQ->p2->tail = to_enque;
	}
	else if (priority == 3){
		ptr_readyQ->p3->tail->next = to_enque;
		ptr_readyQ->p3->tail = to_enque;
	}
}

PCB *deque_PCB_from_readyQ()
{
	PCB *to_return;
	
	if(ptr_readyQ->p0->head !=NULL){
		to_return = ptr_readyQ->p0->head;
		to_return->next = NULL;
		ptr_readyQ->p0->head = ptr_readyQ->p0->head->next;
	}
	
	else if(ptr_readyQ->p1->head !=NULL){
		to_return = ptr_readyQ->p1->head;
		ptr_readyQ->p1->head = ptr_readyQ->p1->head->next;
	}
	
	else if(ptr_readyQ->p2->head !=NULL){
		to_return = ptr_readyQ->p2->head;
		ptr_readyQ->p2->head = ptr_readyQ->p2->head->next;
	}
	
	else if(ptr_readyQ->p3->head !=NULL){
		to_return = ptr_readyQ->p3->head;
		ptr_readyQ->p3->head = ptr_readyQ->p3->head->next;
	}
	
	to_return->next = NULL;
	return to_return;
}

void enque_PCB_to_blocked_on_requestQ(PCB *to_enque)
{
	ptr_blocked_on_requestQ->tail->next = to_enque;
	to_enque->next = NULL;
	ptr_blocked_on_requestQ->tail = to_enque;
}

PCB *deque_PCB_from_blocked_on_requestQ()
{
	PCB* to_return = ptr_blocked_on_requestQ->head;
	ptr_blocked_on_requestQ->head = ptr_blocked_on_requestQ->head->next;
	to_return->next = NULL;

	return to_return;
}

void enque_PCB_to_blocked_on_receiveQ(PCB *to_enque)
{
	ptr_blocked_on_receiveQ->tail->next = to_enque;
	to_enque->next = NULL;
	ptr_blocked_on_receiveQ->tail = to_enque;
}

PCB *deque_PCB_from_blocked_on_receiveQ()
{
	PCB* to_return = ptr_blocked_on_receiveQ->head;
	ptr_blocked_on_receiveQ->head = ptr_blocked_on_receiveQ->head->next;
	to_return->next = NULL;

	return to_return;
	
}

void process_switch()
{
	PCB *next_pcb, *old_pcb;
	next_pcb = deque_PCB_from_readyQ(); //get ptr to highest priority ready process
	next_pcb->status = EXECUTING;
	old_pcb = current_process;
	current_process = next_pcb;
	context_switch(old_pcb->context, next_pcb->context);
}

void context_switch(jmp_buf *previous, jmp_buf *next)
{
	int return_code = setjmp(previous); //save the context of the previous process
	if (return_code == 0) 
		longjmp(next,1); // start the next process from where it left of	
}

void null_process()
{
	while(1)
		K_release_processor();
}
void KB_I_Proc()
{
     MsgEnv * msgsend = K_receive_message();          //allocate msg env to send to cci

     
     
     kb_sm * kb_sm_ptr;        //check kernel.h contains declaration
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
     crt_sm * crt_sm_ptr;           //optional?

     //wait for msg to arrive from cci
     while(msgrecieved == NULL);

     strcpy(crt_sm_ptr->data, msgrecieved->text_area); 
     crt_sm_ptr->status = 0;
     
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
        int temp = TimeoutQ->tail->sender_id;
        send->sender_id = TimeoutQ->tail->sender_id;
        send->type = WAKE_UP;
        TimeoutQ->tail->next = NULL;
        K_send_message(temp, send);
        
    }
    
}


//Clock Process    ----   Kernel Function
      
void clock_proc()
{
     int clock_time=0;
     char buffer[CLOCK];
     char clock[CLOCK]={};
     MsgEnv* msgsend;
     msgsend->type = TIMER_REQUEST;
     
     MsgEnv* msgdisp;
     msgsend->type = DISPLAY_REQUEST;
     
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
         