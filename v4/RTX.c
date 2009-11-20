//hyesun edit
#include "RTX.h"

int main(){
	
	Initialization();
	
	MsgEnv *temp;
	temp = (MsgEnv*)malloc(sizeof(MsgEnv));
	
	temp->env_id=0;
	temp->next=NULL;
	temp->sender_id=0;
	temp->dest_id = 0;
	temp->type = 0;
	temp->num_clock_ticks= 0;
	
	
	//~ int env_id
	//~ struct MsgEnv *next; //used when env is on a queue
	//~ int sender_id;
	//~ int dest_id;
	//~ int type;
	//~ int num_clock_ticks;
	//~ char text_area[TEXT_AREA_SIZE];

	temp = K_request_process_status(temp);
	//~ printf("%s",temp->text_area);
	return 0;
}

