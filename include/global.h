/* Contains global */

/* Variables */
#define SMSIZE 100
#define MAXCHAR 80
#define KB_MAXCHAR 80
#define CRT_MAXCHAR 85
#define SLEEP 500
//~ int SMSIZE = 100;
//~ int MAXCHAR = 80;
//~ int KB_MAXCHAR = MAXCHAR;
//~ int CRT_MAXCHAR = MAXCHAR +5;
//~ int SLEEP = 500;

/* Objects */
typedef struct	//kb shared memory
{
	int status;				//status bit
	char data[KB_MAXCHAR];		//data array
} kb_sm;

typedef struct	//crt shared memory
{
	int status;				//status bit
	char data[CRT_MAXCHAR];	//data array
} crt_sm;

