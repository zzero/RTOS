#ifndef KERNEL_H
#define KERNEL_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>

#define SUCCESS 1
#define FAIL 0
#define TEXT_AREA_SIZE 1000

//process status definition
#define READY 0
#define EXECUTING 1
#define BLOCKED_ON_REQUEST 2
#define BLOCKED_ON_RECEIVE 3

/* Process ID */
#define NUMB_PROC 8
#define PROCESSA 1
#define PROCESSB 2
#define PROCESSC 3
#define NULL_PROCESS 4
#define CCI 5
#define CRT_I_PROC 6
#define TIMER_I_PROC 7
#define KB_I_PROC 8
#define CLOCK_PID 9 //???

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
#define DISPLAY_REQUEST 7   // USED IN CLOCK PROCESS
#define GET_TRACE_BUF 8
//NUMBER OF MSGES
#define NUMB_MSG_ENV 10

/*CLOCK PROCESS CONSTANTS*/   //------  ADDED  
#define CLOCK_ON 1
#define SET_CLOCK 2
#define CLOCK 8

//define number of processes
#define USR_PROC_NUMB 5
#define iPROC_NUMB 3

//define default PID
#define defaultPID -1

//KB and CRT related
#define SMSIZE 100
#define KB_MAXCHAR 80
#define CRT_MAXCHAR 85
#define SLEEP 500



/* Structures */

//kb shared memory
typedef struct
{
	int status;				//status bit
	char data[KB_MAXCHAR];	//data array
}kb_sm;

//crt shared memory
typedef struct
{
	int status;				//status bit
	char data[CRT_MAXCHAR];	//data array
}crt_sm;

//message env
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
	void(*start_PC)(); // address of the instruction being executed, or the address of the next instruction to be executed. Possibly be void pointer to a function? YES, correct me if I am wrong tho. (BK)
	jmp_buf context;
	char *stack_pointer; //CPU register. A stack pointer, usually in the form of a hardware register, points to the most recently referenced location on the stack; when the stack has a size of zero, the stack pointer points to the origin of the stack. Possibly be void pointer?
	int atomic_count; 
	MsgEnv *receive_env_head;
	MsgEnv *receive_env_tail;
	MsgEnv *ip_free_msgQ; //used only by iprocesses to have their own set of free env
	//~ char ip_state[10]; //stores whether iprocess is executing or idle
	int ip_status; //Status of IProc
	struct PCB *next;
}PCB;

//general pcb Q
typedef struct pcbHT
{
	struct PCB *head;
	struct PCB *tail;
}pcbHT;

//general MsgEnv Q
typedef struct MsgEnvHT
{
	struct MsgEnv *head;
	struct MsgEnv *tail;
}MsgEnvHT;

//ready Q
typedef struct readyQ
{
	pcbHT *p0;
	pcbHT *p1;
	pcbHT *p2;
	pcbHT *p3;
}readyQ;

typedef struct trace
{
	int dest_pid;
	int source_pid;
	int message_type;
	int time_stamp; //FIXME: what type?
	struct trace *next;
}trace;

typedef struct sendTrcBfr
{
	trace *sendTrcBfr_head;
	trace *sendTrcBfr_tail;
	int trace_numb;
}sendTrcBfr;

typedef struct recvTrcBfr
{
	trace *recvTrcBfr_head;
	trace *recvTrcBfr_tail;
	int trace_numb;
}recvTrcBfr;

/* Variables */

/*Beomjoon*/
PCB *current_process;

readyQ *ptr_readyQ;
pcbHT *ptr_blocked_on_receiveQ;
pcbHT *ptr_blocked_on_requestQ;

MsgEnv *ptr_free_envQ;

sendTrcBfr *TBsend;
recvTrcBfr *TBreceive;

PCB *timer_i_proc;
PCB *crt_i_proc;
PCB *kb_i_proc;
MsgEnvHT *TimeoutQ;

/* Functions */

/*Beomjoon*/
int K_request_process_status(MsgEnv *msg_env);
int K_change_priority(int new_priority, int target_process_id);

int K_release_msg_env(MsgEnv *msg_env);
MsgEnv *K_request_msg_env();

int K_send_message(int dest_pid, MsgEnv *msg_env);
MsgEnv *K_receive_message();

void trace_send(int sender_id, int dest_id, int type); 
void trace_receive(int sender_id, int dest_id, int type); 

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
PCB *deque_PCB_from_blocked_on_receiveQ(PCB *to_deque);

/*Tope*/
void Initialization();
void enque_PCB_to_readyQ(PCB *to_enque);
PCB *deque_PCB_from_readyQ();
void process_switch();
void context_switch(jmp_buf *previous, jmp_buf *next);
void null_process();
void CRT_I_Proc();
void Timer_I_Proc();
void KB_I_Proc();

/* Hyesun - forking related */
int status;
int kb_pid, kb_sm_fid;
int crt_pid, crt_sm_fid;
char * kb_filename = "kb_sm_file";
char * crt_filename = "crt_sm_file";
caddr_t kb_mmap_ptr, crt_mmap_ptr;
kb_sm * kb_sm_ptr;
crt_sm * crt_sm_ptr;

#endif

