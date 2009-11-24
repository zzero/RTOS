# Project: RTX


CPP  = g++
CC   = gcc

RES  = 
OBJ  = RTX.o kernel.o crt.o keyboard.o $(RES)
LINKOBJ  = kernel.o crt.o keyboard.o $(RES)
LIBS =   
INCS = 
CXXINCS = 

CXXFLAGS = $(CXXINCS)  
CFLAGS = $(INCS)  
RM = rm -f


all: myRTX CRT KB


clean: 
	${RM} $(OBJ) RTX 

myRTX: $(OBJ)
	$(CC) $(LINKOBJ) -g -o "RTX" $(LIBS) -lrt

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

RTX.o: RTX.o
	$(CC) -c -g RTX.c -o RTX.o $(CFLAGS) 
