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
	atomic(0);
	return temp;
}

int release_processor(){
	atomic(1);
	int temp;
	temp = K_release_processor();
	atomic(0);
	return temp;
}

int get_console_chars(MsgEnv *msg_env)
{
	atomic(1);
	int temp;
	temp = K_get_console_chars(msg_env);
	atomic(0);
	return temp;
}

int change_priority(int new_priority, int target_process_id)
{
	atomic(1);
	int temp;
	temp = K_change_priority(new_priority, target_process_id);
	atomic(0);
	return temp;
	
}

int send_message(int dest_pid, MsgEnv *msg_env)
	{
		atomic(1);
		int temp;
		temp = K_send_message(dest_pid, msg_env);
		atomic(0);
		return temp;
	}
MsgEnv *receive_message()
	{
		atomic(1);
		MsgEnv* temp;
		//printf("cur process: %d\n",current_process->pid);
		temp = K_receive_message();
		atomic(0);
		return temp;
	}
int request_process_status(MsgEnv *msg_env)
	{
		atomic(1);
		int temp;
		temp = K_request_process_status(msg_env);
		atomic(0);
		return temp;
	}

int get_trace_buffers(MsgEnv *msg_env)
	{
		atomic(1);
		int temp;
		temp = K_get_trace_buffers(msg_env);
		atomic(0);
		return temp;
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
		//	sigfillset(&newmask);
			
			sigaddset(&newmask, 14); //the alarm signal
			sigaddset(&newmask, 2); // the CNTRL-C
			sigaddset(&newmask, SIGUSR1);
			sigaddset(&newmask, SIGUSR2);
			sigaddset(&newmask, SIGTTIN);
			sigaddset(&newmask, SIGTRAP);
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
{
	while(1){
		//printf("PROCESS A RUNNING\n");
		K_release_processor();
	}
}

void ProcessB()
{	
	while(1){
		//printf("PROCESS B RUNNING\n");
		K_release_processor();
	}
}

void ProcessC()
{	
	while(1){
		//printf("PROCESS C RUNNING\n");
		K_release_processor();
	}
}


void sig_handler(int sig_name)
{
	//printf("INSIDE SIG HANDLER\n");
	
	//~ printf("sigrecevied: %d\n", sig_name);
	//~ printf("SIGUSR1 %d\n", SIGUSR1);
	//~ printf("SIGUSR2: %d\n", SIGUSR2);
	//~ printf("SIGINT: %d\n", SIGINT);
	//~ printf("SIGALARM: %d\n", SIGALRM);
	
	
	/*Disable Signals, save pointer to currently active PCB*/
	PCB* save = current_process;  
	//~ enque_PCB_to_ready
	/*Realize Signal, excecute appropriate action*/
	switch(sig_name){
	
		/*Terminate Signal*/
		case SIGINT:
			terminate();
			break;

		
		/*Timing Signal - service delay requests, complement UALRM*/
		case SIGALRM:  //Alarm Signal (timing)
			current_process ->pid = TIMER_I_PROC;
			current_process->ip_status = EXECUTING;
			Timer_I_Proc();
			current_process->ip_status = IDLE;
			break;

	    
		/*CRT Signal*/
		case SIGUSR2:
			//if(cciHandleCRT == 1){
				printed = 1;	//crt has finished printing
				current_process = crt_i_proc;
				current_process->ip_status = EXECUTING;
				CRT_I_Proc();
				current_process->ip_status = IDLE;
				cciHandleCRT = 0;
			//}
			break; 
			
		/*KB Signal*/
		case SIGUSR1:
			//if(cciHandleKB == 1){
				current_process = kb_i_proc;
				current_process->ip_status = EXECUTING;
				KB_I_Proc();
				current_process->ip_status = IDLE;
				cciHandleKB = 0;
			//}
			break; 

		default:	
		printf("Unknown Signal!\n");
			terminate();
			exit(1);
			break;

    }
    //printf("Done signal handling..\n");
    //~ printf("pid = %x\n\n", save);
    
    current_process = save;    
}    

void Initialization()
{
	//variables required for forking
	int rtx_pid;
	char kb_arg1[20], kb_arg2[20];
	char crt_arg1[20], crt_arg2[20];
	cciHandleKB = 0;
	cciHandleCRT = 0;
	iTableRow *iTable = (struct iTableRow*)malloc(sizeof(iTableRow)*8);
	
	//Write to the iTable
	//Process A
	iTable[0].pid=PROCESSA;
	iTable[0].priority=1;
	iTable[0].stacksize=STACKSIZE;
	void (*fptr)(); 
	fptr = &ProcessA;	
	iTable[0].start_PC=fptr;

	//Process B
	iTable[1].pid=PROCESSB;
	iTable[1].priority=1;
	iTable[1].stacksize=STACKSIZE;
	fptr = &ProcessB;
	iTable[1].start_PC=fptr;

	//Process C
	iTable[2].pid=PROCESSC;
	iTable[2].priority = 1;
	iTable[2].stacksize = STACKSIZE;
	fptr = &ProcessC;
	iTable[2].start_PC=fptr;

	//Null process
	iTable[3].pid= NULL_PROCESS;
	iTable[3].priority = 3;
	iTable[3].stacksize = STACKSIZE;
	fptr = null_process;
	iTable[3].start_PC = fptr;

	//CCI
	iTable[4].pid=5;
	iTable[4].priority=0;
	iTable[4].stacksize=STACKSIZE;
	fptr=cci;
	iTable[4].start_PC=fptr;

	//crt_i_proc
	iTable[5].pid=6;
	iTable[5].priority=0;
	iTable[5].stacksize=STACKSIZE;
	fptr=CRT_I_Proc;
	iTable[5].start_PC=fptr;

	//timer_i_proc
	iTable[6].pid=7;
	iTable[6].priority=0;
	iTable[6].stacksize=STACKSIZE;
	fptr=Timer_I_Proc;
	iTable[6].start_PC=fptr;

	//kb_i_proc
	iTable[7].pid=8;
	iTable[7].priority=0;
	iTable[7].stacksize=STACKSIZE;
	fptr=KB_I_Proc;
	iTable[7].start_PC=fptr;

	int i;
	int j;
	current_process = NULL; //BK
	ptr_readyQ = (readyQ*)malloc(sizeof(readyQ));
	TBsend= (sendTrcBfr*)malloc(sizeof(sendTrcBfr));
	TBreceive = (recvTrcBfr*)malloc(sizeof(recvTrcBfr));
	
	//BK
	/*initialize readyQ*/
	ptr_readyQ->p0 = (pcbHT*)malloc(sizeof(pcbHT));
	ptr_readyQ->p1 = (pcbHT*)malloc(sizeof(pcbHT));
	ptr_readyQ->p2 = (pcbHT*)malloc(sizeof(pcbHT));
	ptr_readyQ->p3 = (pcbHT*)malloc(sizeof(pcbHT));
	
	ptr_readyQ->p0->head = NULL;
	ptr_readyQ->p0->tail = NULL;
	ptr_readyQ->p1->head = NULL;
	ptr_readyQ->p1->tail = NULL;
	ptr_readyQ->p2->head = NULL;
	ptr_readyQ->p2->tail = NULL;
	ptr_readyQ->p3->head = NULL;
	ptr_readyQ->p3->tail = NULL;

	/*initialize trace buffer*/
	TBsend->sendTrcBfr_head = NULL;
	TBsend->sendTrcBfr_tail = NULL; 
	TBreceive->recvTrcBfr_head = NULL;
	TBreceive->recvTrcBfr_tail = NULL;
	
	MsgEnv *msge;
	trace *tracex;
	PCB *apcb;
	
	for(i=1; i<=NUMB_MSG_ENV; i++) //create 10 messsage envelopes and add to message envelope Q
	{		
		msge = (MsgEnv*)malloc(sizeof(MsgEnv));
		msge->type = FREE;		
		msge->env_id = i;
		msge->next = ptr_free_envQ;
		msge->sender_id = defaultPID;
		msge->dest_id = defaultPID;
		ptr_free_envQ = msge;
	}

	for(i=0; i<17; i++) //create 16 trace structs and add to send trace buffer
	//BK: I've made it to seven teen
	{		
		tracex = (trace*)malloc(sizeof(trace));
		tracex->dest_pid = defaultPID;
		tracex-> source_pid = defaultPID;
		tracex->message_type = FREE;
		tracex->time_stamp = 0;


		if(TBsend->sendTrcBfr_head == NULL){
			TBsend->sendTrcBfr_tail = tracex;
			tracex->next=NULL;
		}

		else
			tracex->next = TBsend->sendTrcBfr_head;
	
		TBsend->sendTrcBfr_head = tracex;
		TBsend->trace_numb = 0;
	}
	
	for(i=0; i<17; i++) //create 16 trace structs and add to receive trace buffer
	{		
		tracex = (trace*)malloc(sizeof(trace));
		tracex->dest_pid = defaultPID;
		tracex->source_pid = defaultPID;
		tracex->message_type = FREE;
		tracex->time_stamp = 0;
		
		if(TBreceive->recvTrcBfr_head == NULL){
			TBreceive->recvTrcBfr_tail = tracex;
			tracex->next=NULL;
		}
		else
			tracex->next=TBreceive->recvTrcBfr_head;
		
		TBreceive->recvTrcBfr_head = tracex;
		TBreceive->trace_numb = 0;
	}
	
	
	ptr_blocked_on_requestQ = (pcbHT*)malloc(sizeof(pcbHT));
	ptr_blocked_on_requestQ->head = NULL; //BK
	ptr_blocked_on_requestQ->tail = NULL; //BK
	
	ptr_blocked_on_receiveQ = (pcbHT*)malloc(sizeof(pcbHT));
	ptr_blocked_on_receiveQ->head = NULL; //BK
	ptr_blocked_on_receiveQ->tail = NULL; //BK

	//initialize timing services 
	TimeoutQ=(MsgEnvHT*)malloc(sizeof(MsgEnvHT));
	TimeoutQ->head=NULL;
	TimeoutQ->tail=NULL;

	jmp_buf kernel_buf;
	
	//~ //we set it so that user processes are listed first in the IT
	for (i=0; i<USR_PROC_NUMB; i++) //for each user process, do the following
	{
		apcb = (PCB*)malloc(sizeof(PCB));
		apcb->pid = iTable[i].pid; //FIXME: iTable is not defined in kernel.h
		apcb->priority = iTable[i].priority;
		apcb->stacksize = iTable[i].stacksize;
		apcb->start_PC = iTable[i].start_PC;
		apcb->stack_pointer = (char*)malloc(STACKSIZE)+(STACKSIZE)-SK_OFFSET; 
		apcb->status = READY;
		apcb->ip_status = 1;
		apcb->ip_free_msgQ=NULL;		
		apcb->receive_env_head= NULL;
		apcb->receive_env_tail = NULL;
		enque_PCB_to_readyQ(apcb); //enqueue pcb to the ready queue.
		if (setjmp (kernel_buf) == 0) 
		{
			char *jmpsp;
			jmpsp = apcb->stack_pointer;

			//~ #ifdef _sparc
				//~ _set_sp(jmpsp);
			//~ #endif // ASM??? comment this out....??
			__asm__ ("movl %0,%%esp" :"=m" (jmpsp));
			if (setjmp(apcb->context) == 0) //BK: setjmp function takes the jmp_buf, not the address of it. You should dereference context ptr (BK)
			{
				longjmp (kernel_buf, 1); 
			}
			else
			{
				void (*tmp_fn)();
				tmp_fn=(void*) current_process->start_PC;
				tmp_fn();
			}
		}
		
	}

	//set up PCBs for iprocesses
	for(i = USR_PROC_NUMB; i<(USR_PROC_NUMB+iPROC_NUMB); i++)
	{
		apcb = (PCB*)malloc(sizeof(PCB));
		apcb->pid = iTable[i].pid;
		apcb->priority = iTable[i].priority;
		apcb->stacksize = iTable[i].stacksize;
		apcb->stack_pointer = (char*)malloc(STACKSIZE)+(STACKSIZE);	
		
		apcb->ip_free_msgQ =NULL; //msg envelopes will be added on next line
		for(j = 0; j<10; j++)
		{
			msge = (MsgEnv*)malloc(sizeof(MsgEnv));
			msge->type=0;
			msge->next=apcb->ip_free_msgQ;
			apcb->ip_free_msgQ=msge;
		}
		
		apcb->status = iPROC;
		apcb->ip_status= IDLE;
		apcb->receive_env_head= NULL;
		apcb->receive_env_tail = NULL;
		
		if(i == 5)
			crt_i_proc=apcb;
		if (i == 6)
			timer_i_proc=apcb;
		if(i == 7)			
			kb_i_proc=apcb;	
	}
	sigset (SIGINT, sig_handler); //Where should the terminate() function be?
	sigset (SIGALRM, sig_handler);
	sigset (SIGUSR1, sig_handler);
	sigset (SIGUSR2, sig_handler);
	//ualarm(50000, 50000);
	
	
	/*-----------------------------------------------------FORK-----------------------------------------------------------*/
	kb_filename = "kb_sm_file";
	crt_filename = "crt_sm_file";
	rtx_pid = getpid();

	/*--------------------------------------PARENT CRT SM MAP-----------------------------*/
	printf("\nParent crt sm map\n");
	
	crt_sm_fid = open(crt_filename, O_RDWR | O_CREAT | O_EXCL, (mode_t) 0755 );
	if (crt_sm_fid < 0)
	{
		printf("Bad Open of mmap file <%s>\n", crt_filename);
		terminate();
	}

	status = ftruncate(crt_sm_fid, SMSIZE);
	if (status)
	{
		printf("Failed to ftruncate the file <%s>, status = %d\n", crt_filename, status );
		terminate();
	}

	crt_mmap_ptr = mmap
	(
		(caddr_t) 0,				//memory location; 0 lets OS choose
		SMSIZE,						//bytes to map
		PROT_READ | PROT_WRITE,		//read and write permissions
		MAP_SHARED,					//accessible by another process
		crt_sm_fid,					//which file is associated with mmap
		(off_t) 0					//offset in page frame	//off_t from types.h
	);

	if (crt_mmap_ptr == MAP_FAILED)
	{
		printf("Parent's memory map has failed, about to quit!\n");
		terminate();
	}
	
	crt_sm_ptr = (crt_sm *) crt_mmap_ptr;	//char_sm pointer to the memory mapped
	
	sprintf(crt_arg1, "%d", rtx_pid);
	sprintf(crt_arg2, "%d", crt_sm_fid);
	
	/*-----------------------------------------PARENT KB SM MAP-----------------------------------------*/
	printf("Parent kb sm map\n");
	
	kb_sm_fid = open(kb_filename, O_RDWR | O_CREAT | O_EXCL, (mode_t) 0755 );
	if (kb_sm_fid < 0)
	{
		printf("Bad Open of mmap file <%s>\n", kb_filename);
		terminate();
	}

	status = ftruncate(kb_sm_fid, SMSIZE);
	if (status)
	{
		printf("Failed to ftruncate the file <%s>, status = %d\n", kb_filename, status );
		terminate();
	}
	
	kb_mmap_ptr = mmap
	(
		(caddr_t) 0,				//memory location; 0 lets OS choose
		SMSIZE,						//bytes to map
		PROT_READ | PROT_WRITE,		//read and write permissions
		MAP_SHARED,					//accessible by another process
		kb_sm_fid,					//which file is associated with mmap
		(off_t) 0					//offset in page frame	//off_t from types.h
	);

	if (kb_mmap_ptr == MAP_FAILED)
	{
		printf("Parent's memory map has failed, about to quit!\n");
		terminate();
	}

	kb_sm_ptr = (kb_sm *) kb_mmap_ptr;	//char_sm pointer to the memory mapped
	
	sprintf(kb_arg1, "%d", rtx_pid);
	sprintf(kb_arg2, "%d", kb_sm_fid);
	
	/*--------------------------FORK CRT------------------------------*/
	printf("Fork crt\n");
	crt_pid = fork();
	
	if(crt_pid == 0)
	{
		execl("./CRT", "CRT", crt_arg1, crt_arg2, (char *)0);

		//should not reach here, but if it does, clean up and exit
		printf("crt initialization failed");
		terminate();
	}

	/*--------------------------FORK KB------------------------------*/
	printf("Fork KB\n");
	
	kb_pid = fork();
	if(kb_pid == 0)
	 {
			execl("./KB", "KB", kb_arg1, kb_arg2, (char *)0);

			//should not reach here, but if it does, clean up and exit
			printf("KB initialization failed");
			terminate();
	 }

	/*--------------------------------DONE FORK----------------------------------*/
	current_process = deque_PCB_from_readyQ();
	current_process->status = EXECUTING;
	longjmp ((current_process->context), 1);
}

