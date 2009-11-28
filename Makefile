# Project: myRTX


CPP  = g++
CC   = gcc

RES  = 
OBJ  = kernel.o RTX.o$(RES)
LINKOBJ  = kernel.o RTX.o$(RES)
LIBS =   
INCS = 
CXXINCS = 

CXXFLAGS = $(CXXINCS)  
CFLAGS = $(INCS)  
RM = rm -f


all: myRTX CRT KB

kill:
	killall -9 KB CRT
cur:
	${RM} kb_sm_file crt_sm_file

clean: 
	${RM} $(OBJ) myRTX CRT KB kb_sm_file crt_sm_file *.o

myRTX: $(OBJ)
	$(CC) $(LINKOBJ) -g -o "myRTX" $(LIBS) -lrt

CRT: crt.o
	$(CC) crt.o -g -o "CRT" -lrt

KB: keyboard.o
	$(CC) keyboard.o -g -o "KB" -lrt

kernel.o: kernel.c
	$(CC) -c -g kernel.c -o kernel.o $(CFLAGS)

crt.o: crt.c
	$(CC) -c -g crt.c -o crt.o $(CFLAGS)

keyboard.o: keyboard.c
	$(CC) -c -g keyboard.c -o keyboard.o $(CFLAGS)
	
RTX.o: RTX.c
	$(CC) -c -g RTX.c -o RTX.o $(CFLAGS)
