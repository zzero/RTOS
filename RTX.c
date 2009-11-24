 #include "include/RTX.h"



int main()
{
	Initialization();
	
	return 0;
}

void ProcessA()
{}

void ProcessB()
{}

void ProcessC()
{}

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

void Initialization()
{
	
	iTableRow *iTable=(struct iTableRow*)malloc(sizeof(iTableRow)*8);

	//Write to the iTable
	//Process A
	iTable[0].pid=1;
	iTable[0].priority=1;
	iTable[0].stacksize=STACKSIZE;
	iTable[0].start_PC=(void(*)())ProcessA;
	
	//Process B
	iTable[1].pid=2;
	iTable[1].priority=1;
	iTable[1].stacksize=STACKSIZE;
	//int (*fptr)();
	//fptr=ProcessB();
	iTable[1].start_PC=(void(*)())ProcessB;

	//Process C
	iTable[2].pid=3;
	iTable[2].priority=1;
	iTable[2].stacksize=STACKSIZE;
	//int (*fptr)();
	//fptr=ProcessC();
	iTable[2].start_PC=(void(*)())ProcessC;

	//Null process
	iTable[3].pid=4;
	iTable[3].priority=3;
	iTable[3].stacksize=STACKSIZE;
	//int (*fptr)();
	//fptr=null_process;
	iTable[3].start_PC=(void(*)())null_process;

	//CCI
	iTable[4].pid=5;
	iTable[4].priority=0;
	iTable[4].stacksize=STACKSIZE;
	//int (*fptr)();
	//fptr=CCI();
	iTable[4].start_PC=(void(*)())CCI;

	//crt_i_proc
	iTable[5].pid=6;
	iTable[5].priority=0;
	iTable[5].stacksize=STACKSIZE;
	//int (*fptr)();
	//fptr=CRT_I_Proc;
	iTable[5].start_PC=(void(*)())CRT_I_Proc;

	//timer_i_proc
	iTable[6].pid=7;
	iTable[6].priority=0;
	iTable[6].stacksize=STACKSIZE;
	//int (*fptr)();
	//fptr=Timer_I_Proc;
	iTable[6].start_PC=(void(*)())Timer_I_Proc;

	//kb_i_proc
	iTable[7].pid=8;
	iTable[7].priority=0;
	iTable[7].stacksize=STACKSIZE;
	//int (*fptr)();
	//fptr=KB_I_Proc;
	iTable[7].start_PC=(void(*)())KB_I_Proc;



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
	
	
	for(i=0; i<10; i++) //create 10 messsage envelopes and add to message envelope Q
	{		
		msge = (MsgEnv*)malloc(sizeof(MsgEnv));
		msge->type = FREE;		
		msge->next = ptr_free_envQ;		
		ptr_free_envQ = msge;
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
	TimeoutQ=(MsgEnvHT*)malloc(sizeof(MsgEnvHT));
	TimeoutQ->head=NULL;
	TimeoutQ->tail=NULL;

	jmp_buf kernel_buf;
	
	//we set it so that user processes are listed first in the IT
	for (i=0; i<USR_PROC_NUMB; i++) //for each user process, do the following
	{
		apcb = (PCB*)malloc(sizeof(PCB));
		apcb->pid = iTable[i].pid; //FIXME: iTable is not defined in kernel.h
		apcb->priority = iTable[i].priority;
		apcb->stacksize = iTable[i].stacksize;
		apcb->start_PC = iTable[i].start_PC;
		apcb->stack_pointer = (char*)malloc(STACKSIZE)+(STACKSIZE)-SK_OFFSET; 
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
				tmp_fn=(void*) current_process->start_PC;
				tmp_fn();
				//current_process->start_PC();
			}
		}

	}

	//set up PCBs for iprocesses
	//~ for(i=3; i<6; i++) //FIXME: try not to use numbers like this.. try to define it.
	for(i = USR_PROC_NUMB; i<iPROC_NUMB; i++)
	{
		apcb = (PCB*)malloc(sizeof(PCB));
		apcb->pid = iTable[i].pid;
		apcb->priority = iTable[i].priority;
		apcb->stacksize = iTable[i].stacksize;
		apcb->stack_pointer = (char*)malloc(STACKSIZE)+(STACKSIZE);	
		
		apcb->ip_free_msgQ =NULL; //msg envelopes will be added on next line
		for(i=0;i<10;i++)
		{
			msge = (MsgEnv*)malloc(sizeof(MsgEnv));
			msge->type=0;
			msge->next=apcb->ip_free_msgQ;
			apcb->ip_free_msgQ=msge;
		}
		
		//~ set status of PCB to 'I_PROCESS'
		apcb->status = iPROC;
		apcb->ip_status= IDLE;
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

	deque_PCB_from_readyQ();
}