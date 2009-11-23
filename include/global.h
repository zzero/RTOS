/* Variables */
#define SMSIZE 100
#define KB_MAXCHAR 80
#define CRT_MAXCHAR 85
#define SLEEP 500

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

