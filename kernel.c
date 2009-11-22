#include "include/kernel.h"

#define STACKSIZE 16384 //whats this

PCB *PCB_finder(int pid){
	PCB *temp;
	
	//~ for (temp = readyQ->p0_head; temp!=NULL; temp = temp->next){
		//~ if (temp->pid==pid)
			//~ return temp;
	//~ }
	
	//~ for (temp = readyQ->p1_head; temp!=NULL; temp = temp->next){
		//~ if (temp->pid==pid)
			//~ return temp;
	//~ }
	
	//~ for (temp = readyQ->p2_head; temp!=NULL; temp = temp->next){
		//~ if (temp->pid==pid)
			//~ return temp;
	//~ }
	
	//~ for (temp = readyQ->p3_head; temp!=NULL; temp = temp->next){
		//~ if (temp->pid==pid)
			//~ return temp;
	//~ }
	for (temp = ptr_readyQ->p0->head; temp!=NULL; temp=temp->next){
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

MsgEnv* K_request_process_status(MsgEnv *msg_env)
{
	PCB *temp;
	char *buffer;
	char *pid = "PID: ";
	char *status = "status: ";
	char *priority = "priority: ";
	int num_proc = 0;
	
	for (temp = ptr_blocked_on_requestQ->head; temp!=NULL; temp = temp->next){
		strcat (pid,itoa(temp->pid,buffer,10));
		strcat (pid,",");
		strcat (status, itoa(temp->status,buffer,10));
		strcat (status,",");
		strcat (priority, itoa(temp->priority,buffer,10));
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
	
	if(ptr_blocked_on_requestQ->head != NULL){
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
	
	while(ptr_free_envQ ==NULL){
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
	
	else{
		enque_msg_to_PCB(msg_env, dest_pcb);
		
		if(dest_pcb->status = BLOCKED_ON_RECEIVE){
			enque_PCB_to_readyQ(dest_pcb);
		}
	}
	
	trace_send(msg_env->sender_id, msg_env->dest_id, msg_env->type);
		
	return SUCCESS;
}

MsgEnv *K_receive_message()
{
	while(current_process->receive_env_head == NULL){
		if(current_process->status = iPROC)
			return NULL;
		
		else{
			current_process->status = BLOCKED_ON_RECEIVE; 	
			enque_PCB_to_blocked_on_receiveQ(current_process);
			process_switch();
		}
	}
	
	MsgEnv *msg_env = deque_msg_from_PCB(current_process);
	trace_receive(msg_env->sender_id, msg_env->dest_id, msg_env->type);
	
	return msg_env;
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
	
	for(send_temp = TBsend->sendTrcBfr_head; send_temp != NULL; send_temp=send_temp->next){
		strcat(sendtrc_buf, itoa(send_temp->dest_pid, buf, 10));
		strcat(sendtrc_buf,",");
		strcat(sendtrc_buf,  itoa(send_temp->source_pid, buf, 10));
		strcat(sendtrc_buf,",");
		strcat(sendtrc_buf,  itoa(send_temp->message_type, buf, 10));
		strcat(sendtrc_buf,",");
		strcat(sendtrc_buf, itoa(send_temp->time_stamp, buf, 10));
		strcat(sendtrc_buf,",\n");
	}
	
	for(receive_temp = TBreceive->recvTrcBfr_head; receive_temp != NULL; receive_temp = receive_temp->next){
		strcat(rcvtrc_buf, itoa(receive_temp->dest_pid,buf,10));
		strcat(rcvtrc_buf,",");
		strcat(rcvtrc_buf, itoa(receive_temp->source_pid,buf,10));
		strcat(rcvtrc_buf,",");
		strcat(rcvtrc_buf, itoa(receive_temp->message_type,buf,10));
		strcat(rcvtrc_buf,",");
		strcat(rcvtrc_buf, itoa(receive_temp->time_stamp,buf,10));
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
	strcpy(msg_env->text_area,itoa(wakeup_code,buf,10));
	msg_env->dest_id = current_process->pid;
	
	int return_value = K_send_message(current_process->pid, msg_env);
	
	return return_value;
}

/*Queues*/
void enque_msg_to_PCB(MsgEnv *msg_env, PCB* dest){
	dest->receive_env_tail->next = msg_env;
	msg_env->next = NULL;
	dest->receive_env_tail = msg_env;
}

MsgEnv *deque_msg_from_PCB(PCB* dest){
	MsgEnv *to_return = dest->receive_env_head;
	dest->receive_env_head = dest->receive_env_head->next;
	to_return->next=NULL;
	
	return to_return;
}

void enque_msg_to_free_envQ(MsgEnv *msg_env){
	msg_env->next = ptr_free_envQ->tail;
	ptr_free_envQ->tail = msg_env;
}

MsgEnv* deque_msg_from_free_envQ(){
}
	
void enque_PCB_to_readyQ(PCB *to_enque){
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

PCB *deque_PCB_from_readyQ(){
	PCB *to_return;
	
	if(ptr_readyQ->p0->head !=NULL){
		to_return = ptr_readyQ->p0->head;
		to_return->next = NULL;
		ptr_readyQ->p0->head = ptr_readyQ->p0->head->next;
	}
	
	else if(ptr_readyQ->p1->head !=NULL){
		to_return = ptr_readyQ->p0->head;
		to_return->next = NULL;
		ptr_readyQ->p0->head = ptr_readyQ->p0->head->next;
	}
	
	else if(ptr_readyQ->p2->head !=NULL){
		to_return = ptr_readyQ->p0->head;
		to_return->next = NULL;
		ptr_readyQ->p0->head = ptr_readyQ->p0->head->next;
	}
	
	else if(ptr_readyQ->p3->head !=NULL){
		to_return = ptr_readyQ->p0->head;
		to_return->next = NULL;
		ptr_readyQ->p0->head = ptr_readyQ->p0->head->next;
	}
	
	return to_return;
	
}

void enque_PCB_to_blocked_on_requestQ(PCB *to_enque){
}

PCB *deque_PCB_from_blocked_on_requestQ(){
	
	PCB* to_return = ptr_blocked_on_requestQ->head;
	ptr_blocked_on_requestQ->head = ptr_blocked_on_requestQ->head->next;
	to_return->next = NULL;
	
	return to_return;
}

void enque_PCB_to_blocked_on_receiveQ(PCB *to_enque)
{}
PCB *deque_PCB_from_blocked_on_receiveQ()
{}

void process_switch()
{
	//~ PCB *next_pcb, *old_pcb;
	//~ next_pcb = rpq_dequeue( ); //get ptr to highest priority ready process
	//~ next_pcb->status = EXECUTING;
	//~ old_pcb = current_process;
	//~ current_process = next_pcb;
	//~ context_switch( old_pcb->context, next_pcb->context);
}

void context_switch(jmp_buf *previous, jmp_buf *next)
{
	//~ return_code = setjmp(previous); //save the context of the previous process
	//~ if (return_code == 0) 
	//~ {
		//~ longjmp(next,1); // start the next process from where it left of
	//~ }	
}

void Initialization()
{
	int i; //BK: What's this?
	current_process = NULL; //BK
	ptr_readyQ = (readyQ*)malloc(sizeof(readyQ));
	TBsend= (sendTrcBfr*)malloc(sizeof(sendTrcBfr));
	TBreceive = (recvTrcBfr*)malloc(sizeof(recvTrcBfr));
	
	//BK
	TBsend->sendTrcBfr_head = NULL;
	TBsend->sendTrcBfr_tail = NULL; 
	TBreceive->recvTrcBfr_head = NULL;
	TBreceive->recvTrcBfr_tail = NULL;
	
	MsgEnv *msge;
	trace *tracex;
	PCB *apcb;
	
	ptr_free_envQ->head = NULL; //BK: Needs to be intialized
	ptr_free_envQ->tail = NULL;
	for(i=0; i<10; i++) //create 10 messsage envelopes and add to message envelope Q
	{		
		msge = (MsgEnv*)malloc(sizeof(MsgEnv));
		msge->type = FREE;
		//~ msge->next = ptr_free_envQ; //FIXME: should we create a actual Q for this?
		msge->next = ptr_free_envQ->head;
		//~ ptr_free_envQ = msge;
		ptr_free_envQ->head = msge;
	}

	for(i=0; i<16; i++) //create 16 trace structs and add to send trace buffer
	{		
		tracex = (trace*)malloc(sizeof(trace));
		
		//~ if(TB->sendTrcBfr_head == NULL)
		if(TBsend->sendTrcBfr_head == NULL)
			TBsend->sendTrcBfr_tail = tracex;
		
		//~ trace->next=TBsend->sendTrcBfr_head;
		tracex->next = TBsend->sendTrcBfr_head;
		TBsend->sendTrcBfr_head = tracex;
	}
	
	for(i=0; i<16; i++) //create 16 trace structs and add to receive trace buffer
	{		
		tracex = (trace*)malloc(sizeof(trace));
		
		if(TBreceive->recvTrcBfr_head == NULL)
			TBreceive->recvTrcBfr_tail = tracex;
		
		tracex->next=TBreceive->recvTrcBfr_head;
		TBreceive->recvTrcBfr_head = tracex;

	}
	
	ptr_blocked_on_requestQ = (pcbHT*)malloc(sizeof(pcbHT));
	ptr_blocked_on_requestQ->head = NULL; //BK
	ptr_blocked_on_requestQ->tail = NULL; //BK
	
	ptr_blocked_on_receiveQ = (pcbHT*)malloc(sizeof(pcbHT));
	ptr_blocked_on_receiveQ->head = NULL; //BK
	ptr_blocked_on_receiveQ->tail = NULL; //BK
	//initialize timing services 

	jmp_buf kernel_buf;
	/*
	//we assume the user processes are listed first in the IT
	for (i=0; i<USR_PROC_NUMB; i++) //for each user process, do the following
	{
		apcb = (PCB*)malloc(sizeof(PCB));
		apcb->pid = iTable[i]->pid; //FIXME: iTable is not defined in kernel.h
		apcb->priority = iTable[i]->priority[i];
		apcb->stacksize = iTable[i]->stacksize[i];
		apcb->start_PC = iTable[i]->start_PC[i];
		apcb->stack_pointer = (*char)malloc(apcb->stacksize)+apcb->stacksize; 
		//~ apcb->status = "ready"; //BK
		apcb->status = READY;
		apcb->ip_status = 1;
		apcb->ip_free_msgQ=NULL;		
		apcb->receive_env_head= NULL;
		apcb->receive_env_tail = NULL;
		enque_pcb(apcb); //enqueue pcb to the ready queue.
		//FIXME: We don't even have a function header for this

		if (setjmp (kernel_buf) == 0) 
		{
			char *jmpsp = apcb->stack_pointer;

			#ifdef _sparc
			_set_sp(jmpsp);
			#endif
			if (setjmp (apcb->context) == 0) 
			{
				longjmp (kernel_buf, 1); 
			}
			else
			{
				void (*tmp_fn)();
				tmp_fn=(void*) current_process_start_PC;
				tmp_fn();
			}
		}

	}

	//set up PCBs for iprocesses
	//~ for(i=3; i<6; i++) //FIXME: try not to use numbers like this.. try to define it.
	for(i = USR_PROC_NUMB; i<iPROC_NUMB; i++)
	{
		apcb = (PCB*)malloc(sizeof(PCB));
		apcb->pid = iTable[i]->pid;
		apcb->priority = iTable[i]->priority[i];
		apcb->stacksize = iTable[i]->stacksize[i];
		apcb->stack_pointer = (*char)malloc(apcb->stacksize)+apcb->stacksize;	
		
		apcb->ip_free_msgQ =NULL //msg envelopes will be added on next line
		msge = (MsgEnv*)malloc(sizeof(MsgEnv));
		msge->type=0;
		msge->next=apcb->ip_free_msgQ;
		apcb->ip_free_msgQ=msge;
		
		//~ set status of PCB to 'I_PROCESS'
		apcb->status = iPROC;
		apcb->ip_status= IDLE
		apcb->receive_env_head= NULL;
		apcb->receive_env_tail = NULL;
		
		if(i == 3)
			timer_i_proc=apcb;
		if (i == 4)
			crt_i_proc=apcb;
		if(i == 5)			
			kb_i_proc=apcb;

		
	}

	sigset (SIGINT, terminate()); //Where should the terminate() function be?
	sigset (SIGALRM, timer_i_proc);
	sigset (SIGUSR1, crt_i_proc);
	sigset (SIGUSR2, kb_i_proc);
	ualarm(10000, 10000);
	*/
	
	//Forking KB and CRT
	

	/* Variable */
	
	//~ int rtx_pid;
	//~ int status;

	//~ char * kb_filename = "kb_sm_file";
	//~ char * crt_filename = "crt_sm_file";

	//~ int kb_sm_fid;
	//~ int crt_sm_fid;

	//~ int crt_pid;
	//~ int kb_pid;

	//~ /* Fork */
	//~ rtx_pid = getpid();

	//~ /* CRT fork */
	//~ crt_sm_fid = open(crt_filename, O_RDWR | O_CREAT | O_EXCL, (mode_t) 0755 );

	//~ if (crt_sm_fid < 0)
	//~ {
		//~ printf("Bad Open of mmap file <%s>\n", crt_filename);
		//~ terminate();
	//~ }

	//~ status = ftruncate(crt_sm_fid, SMSIZE);
	//~ if (status)
	//~ {
		//~ printf("Failed to ftruncate the file <%s>, status = %d\n", crt_filename, status );
		//~ terminate();
	//~ }

	//~ //use fork to duplicate current process
	//~ crt_pid = fork();

	//~ if(crt_pid == 0)	//then this is child
	//~ {
		//~ execl("./crt", "crt.c", rtx_pid, crt_sm_fid);
		
		//~ //should not reach here, but if it does, clean up and exit
		//~ printf("CRT initialization failed");
		//~ terminate();
	//~ }

	//~ //if not, process is parent
	//~ sleep(1);	//to allow crt to execute

	//~ //need pointer to the crt_sm

	//~ //memory map
	//~ mmap_ptr = mmap
	//~ (
		//~ (caddr_t) 0,				//memory location; 0 lets OS choose
		//~ SMSIZE,						//bytes to map
		//~ PROT_READ | PROT_WRITE,		//read and write permissions
		//~ MAP_SHARED,					//accessible by another process
		//~ crt_sm_fid,					//which file is associated with mmap
		//~ (off_t) 0					//offset in page frame	//off_t from types.h
	//~ );

	//~ if (mmap_ptr == MAP_FAILED)
	//~ {
		//~ printf("Parent's memory map has failed, about to quit!\n");
		//~ terminate();
	//~ }

	//~ crt_sm_ptr = (crt_sm *) mmap_ptr;	//sm pointer to the memory mapped


	//~ /* KB fork */
	//~ kb_sm_fid = open(kb_filename, O_RDWR | O_CREAT | O_EXCL, (mode_t) 0755 );

	//~ if (kb_sm_fid < 0)
	//~ {
		//~ printf("Bad Open of mmap file <%s>\n", kb_filename);
		//~ terminate();
	//~ }

	//~ status = ftruncate(kb_sm_fid, SMSIZE);
	//~ if (status)
	//~ {
		//~ printf("Failed to ftruncate the file <%s>, status = %d\n", kb_filename, status );
		//~ terminate();
	//~ }

	//~ //use fork to duplicate current process
	//~ kb_pid = fork();

	//~ if(kb_pid == 0)
	//~ {
		//~ execl("./keyboard", "keyboard.c", rtx_pid, kb_sm_fid);

		//~ //should not reach here, but if it does, clean up and exit
		//~ printf("KB initialization failed");
		//~ terminate();
	//~ }

	//~ //if not, process is parent
	//~ sleep(1);	//to allow kb to execute

	//~ //need pointer to the kb_sm

	//~ //memory map
	//~ mmap_ptr = mmap
	//~ (
		//~ (caddr_t) 0,				//memory location; 0 lets OS choose
		//~ SMSIZE,						//bytes to map
		//~ PROT_READ | PROT_WRITE,		//read and write permissions
		//~ MAP_SHARED,					//accessible by another process
		//~ kb_sm_fid,					//which file is associated with mmap
		//~ (off_t) 0					//offset in page frame	//off_t from types.h
	//~ );

	//~ if (mmap_ptr == MAP_FAILED)
	//~ {
		//~ printf("Parent's memory map has failed, about to quit!\n");
		//~ terminate();
	//~ }

	//~ kb_sm_ptr = (kb_sm *) mmap_ptr;	//sm pointer to the memory mapped

	
}


					


