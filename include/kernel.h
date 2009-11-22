#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>

#ifndef KERNEL_H
#define KERNEL_H

#define SUCCESS 1
#define FAIL 0
#define TEXT_AREA_SIZE 100

//process status definition
#define READY 0
#define EXECUTING 1
#define BLOCKED_ON_REQUEST 2
#define BLOCKED_ON_RECEIVE 3
//below are used by iproc only
//iPROC process ID
#define iPROC 100
//status of iproc
#define IDLE -1
#define RUNNING 0

//msg type definition
#define FREE 0
#define COUNT_REPORT 1 // for user procs
#define WAKE_UP 2
#define TIMER_REQUEST 3
#define DISPLAY_CLOCK 4
#define DISPLAY_ACKNOWLEDGEMENT 5
#define CONSOLE_INPUT 6

//define number of processes
#define USR_PROC_NUMB 3
#define iPROC_NUMB 6

//define default PID
#define defaultPID -1
/*Structures*/

typedef struct MsgEnv
{
	int env_id;
	struct MsgEnv *next; //used when env is on a queue
	int sender_id;
	int dest_id;
	int type;
	int num_clock_ticks;
	char text_area[TEXT_AREA_SIZE];
}MsgEnv;

//PCB definitions
typedef struct PCB
{
	//~ char status[10]; //ready BReceive BRequest executing
	int status;
	int pid;
	int priority;
	int stacksize;
	char *start_PC; // address of the instruction being executed, or the address of the next instruction to be executed. Possibly be void pointer?
	jmp_buf context;
	char *stack_pointer; //CPU register. A stack pointer, usually in the form of a hardware register, points to the most recently referenced location on the stack; when the stack has a size of zero, the stack pointer points to the origin of the stack. Possibly be void pointer?
	int atomic_count; 
	MsgEnv *receive_env_head;
	MsgEnv *receive_env_tail;
	MsgEnv *ip_free_msgQ; //used only by iprocesses to have their own set of free env
	//~ char ip_state[10]; //stores whether iprocess is executing or idle
	int ip_status; //Status of IProc
	PCB *next;

}PCB;

//general pcb Q
typedef struct pcbHT
{
	struct PCB *head;
	struct PCB *tail;
}pcbHT;

//ready Q
typedef struct readyQ
{
	//~ PCB *p0_head;
	//~ PCB *p0_tail;
	//~ PCB *p1_head;
	//~ PCB *p1_tail;
	//~ PCB *p2_head;
	//~ PCB *p2_tail;
	//~ PCB *p3_head;
	//~ PCB *p3_tail;
	pcbHT *p0;
	pcbHT *p1;
	pcbHT *p2;
	pcbHT *p3;
	
	}readyQ;

typedef struct free_envQ{
	MsgEnv *head;
	MsgEnv *tail;
}free_envQ;

typedef struct trace
{
	int dest_pid;
	int source_pid;
	int message_type;
	int time_stamp; //FIXME: what type?
	struct trace *prev;
	struct trace *next;
	}trace;

typedef struct sendTrcBfr
{
	trace *sendTrcBfr_head;
	trace *sendTrcBfr_tail;
}sendTrcBfr;

typedef struct recvTrcBfr
{
	trace *recvTrcBfr_head;
	trace *recvTrcBfr_tail;
}recvTrcBfr;

/*Variables*/
/*Beomjoon*/
PCB *current_process;

readyQ *ptr_readyQ;
pcbHT *ptr_blocked_on_receiveQ;
pcbHT *ptr_blocked_on_requestQ;

free_envQ *ptr_free_envQ;

sendTrcBfr *TBsend;
recvTrcBfr *TBreceive;

PCB *timer_i_proc;
PCB *crt_i_proc;
PCB *kb_i_proc;

/*functions*/
/*Beomjoon*/
MsgEnv* K_request_process_status(MsgEnv *msg_env);//FIXME: I NEED TO SEND MSG NOT RETURN!
int K_change_priority(int new_priority, int target_process_id);

int K_release_msg_env(MsgEnv *msg_env);
MsgEnv *K_request_msg_env();

int K_send_message(int dest_pid, MsgEnv *msg_env);
MsgEnv *K_receive_message();

void trace_send(int sender_id, int dest_id, int type); //To be implemented
void trace_receive(int sender_id, int dest_id, int type); //To be implemented

int K_send_console_chars(MsgEnv *msg_env);
int K_get_console_chars(MsgEnv *msg_env);
int K_get_trace_buffers(MsgEnv *msg_env);

int K_release_processor();
int K_request_delay(int time_delay, int wakeup_code, MsgEnv *msg_env);
PCB *PCB_finder(int pid);

void enque_msg_to_PCB(MsgEnv *msg_env, PCB* dest);
MsgEnv *deque_msg_from_PCB(PCB* dest);

void enque_PCB_to_readyQ(PCB *to_enque);
PCB *deque_PCB_from_readyQ();

void enque_msg_to_free_envQ(MsgEnv *msg_env);
MsgEnv* deque_msg_from_free_envQ();

void enque_PCB_to_blocked_on_requestQ(PCB* to_enque);
PCB *deque_PCB_from_blocked_on_requestQ();

void enque_PCB_to_blocked_on_receiveQ(PCB *to_enque);
PCB *deque_PCB_from_blocked_on_receiveQ();
/*Tope*/

void Initialization();
void enque_pcb(PCB *pcb_ptr);
void rpq_dequeue();
void process_switch();
void context_switch(jmp_buf *previous, jmp_buf *next);
void null_process();

#endif

