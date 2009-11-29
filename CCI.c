#include "include/RTX.h"

/*Command Console Interface*/
void cci()
{
     /* Output CCI: to display wait for acknowledge, send msg 
     to kb_i_proc to return latest keyboard input*/  
	MsgEnv* CCI_env;
	CCI_env = deque_msg_from_free_envQ();     

	while(1){	
	     printf("Inside CCI\n");
	     CCI_env->sender_id = CCI;
	     strcpy(CCI_env->text_area,"CCI: \0");
	      //printf("Sening msg to crt_I_prc\n");
	     
	     send_console_chars(CCI_env);
	     CCI_env =  receive_message();
	     while (CCI_env->type != DISPLAY_ACKNOWLEDGEMENT){
	    	enque_msg_to_PCB(CCI_env, current_process);
		CCI_env = receive_message();
	     }

	     get_console_chars(CCI_env); //sends msg to kb i proc
	     CCI_env =  receive_message();       // ---- line added s
	     while (CCI_env->type != CONSOLE_INPUT){
		enque_msg_to_PCB(CCI_env, current_process);
		CCI_env = receive_message();
	     }

	     /*Store text of CCI_env into character string, convert to lowercase*/
	     char CCI_store[10];
	     strcpy(CCI_store,CCI_env->text_area);
	    // stolower(CCI_store);
	     
	     /*send message envelope to User Process A*/
	     if(CCI_store[0] == 's' && CCI_store[1] == '\0')
	     {
		strcpy(CCI_env->text_area,CCI_store); 
		send_message(PROCESSA, CCI_env); // 1 -> refers to USERPROC_A
		
		send_console_chars(CCI_env);
		CCI_env =  K_receive_message();
		while (CCI_env->type != DISPLAY_ACKNOWLEDGEMENT){
			enque_msg_to_PCB(CCI_env, current_process);
			CCI_env = receive_message();
		}
	     }
	     
	     /*Display Process Status of all processes*/
	     else if(CCI_store[0] == 'p' && CCI_store[1] == 's' && CCI_store[2] == '\0' )
	     {
		strcpy(CCI_env->text_area,CCI_store); 
		request_process_status(CCI_env);
		
		send_console_chars(CCI_env);
		CCI_env =  K_receive_message();
		while (CCI_env->type != DISPLAY_ACKNOWLEDGEMENT){
			enque_msg_to_PCB(CCI_env, current_process);
			CCI_env = receive_message();
		}
	     }
	     
	     //~ /*Sets CCI wall clock to desired format*/
	     //~ else if(CCI_store[0] == 'c' && CCI_store[1] == ' ' && CCI_store[2] == 'h' 
		     //~ && CCI_store[3] == 'h' && CCI_store[4] == ':' && CCI_store[5] == 'm' && CCI_store[6] == 'm'
		     //~ && CCI_store[7] == ':' && CCI_store[8] == 's' && CCI_store[9] == 's' && CCI_store[10] == '\0')
	     //~ {
		 //~ strcpy(CCI_env->text_area,CCI_store);
		 
		 //~ send_console_chars(CCI_env);
		 //~ CCI_env =  K_receive_message();
		 //~ while (CCI_env->type != DISPLAY_ACKNOWLEDGEMENT);
		     
		 //~ MsgEnv* CCI_clock;    
		 //~ CCI_clock->type = SET_CLOCK;  //declare SET_CLOCK as global
		 //~ strcpy(CCI_env->text_area,CCI_store);
		 //~ send_message(CLOCK_PID, CCI_clock); //define CLOCK_pid as global
		 
	     //~ }
	     
	     //~ /*Display wall clock*/
	     //~ else if(CCI_store[0] == 'c' && CCI_store[1] == 'd' && CCI_store[2] == '\0')
	     //~ {
		 //~ MsgEnv* CCI_clock;    
		 //~ CCI_clock->type = CLOCK_ON;  //declare SET_CLOCK as global
		 //~ strcpy(CCI_env->text_area,CCI_store);
		 //~ send_message(CLOCK_PID, CCI_clock); //define CLOCK_pid as global
		 
	     //~ }
	     
	     //~ /*Clear wall clock*/
	     //~ else if(CCI_store[0] == 'c' && CCI_store[1] == 't' && CCI_store[2] == '\0')
	     //~ {
		 //~ strcpy(CCI_env->text_area," ");
		 
		//~ send_console_chars(CCI_env);
		//~ CCI_env =  K_receive_message();
		//~ while (CCI_env->type != DISPLAY_ACKNOWLEDGEMENT);
		
	     //~ }
	     
	     /*Display contents of send and recieve trace buffers*/
	     else if(CCI_store[0] == 'b' && CCI_store[1] == '\0')
	     {
		 strcpy(CCI_env->text_area,CCI_store);
		 
		 get_trace_buffers(CCI_env);
		 printf("CCI_ENV CONTENT = %s\n", CCI_env->text_area);
		 
		 send_console_chars(CCI_env);
		 CCI_env =  K_receive_message();
		 while (CCI_env->type != DISPLAY_ACKNOWLEDGEMENT){
			enque_msg_to_PCB(CCI_env, current_process);
			CCI_env = receive_message();
		}
		 
	     }
	     
	     //~ /*Terminate RTX*/
	     //~ else if(CCI_store[0] == 't' && CCI_store[1] == '\0')
	     //~ {
		 //~ strcpy(CCI_env->text_area,CCI_store);
		 
		 //~ send_console_chars(CCI_env);
		 //~ CCI_env =  K_receive_message();
		 //~ while (CCI_env->type != DISPLAY_ACKNOWLEDGEMENT);
		 
		 //~ sig_handler(SIGINT);
			     
	     //~ }
	     
	     //~ /*Change priority of a specified process*/
	     //~ else if( CCI_store[0] == 'n' && CCI_store[1] == ' ' && isdigit(CCI_store[2]) && CCI_store[3] == ' ' && isdigit(CCI_store[4]) 
		      //~ && CCI_store[5] == '\0' )
	     //~ {
		 //~ strcpy(CCI_env->text_area,CCI_store);
		 //~ int priority, id;
		 //~ const char *pri = CCI_store[2];
		 //~ const char *ID = CCI_store[4]; 
		 //~ priority = atoi(pri);
		 //~ id = atoi(ID);
		 //~ printf("%c, %d, %d\n", CCI_store[0], priority, id);
		 //~ printf("%c, %c, %c\n", CCI_store[0], CCI_store[2], CCI_store[4]);  
	    //~ //     change_priority(priority, id);
		 
	    //~ //     send_console_chars(CCI_env);
	    //~ //     CCI_env =  K_receive_message();
	    //~ //     while (CCI_env->type != DISPLAY_ACKNOWLEDGEMENT);
		 
	     //~ }
	     
	     //~ /*User enters blank space*/
	     //~ else if(CCI_store[0] == ' ' && CCI_store[1] == '\0')   /// replace " "  with ' ' 
	     //~ {
		 //~ strcpy(CCI_env->text_area,CCI_store);
		 
		 //~ send_console_chars(CCI_env);
		 //~ CCI_env =  K_receive_message();
		 //~ while (CCI_env->type != DISPLAY_ACKNOWLEDGEMENT);
		 
	      //~ }
	      release_processor();
      }
		   
}
