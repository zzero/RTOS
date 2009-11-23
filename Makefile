
## Created by Anjuta

CC = gcc
CFLAGS = -g -Wall
OBJECTS = RTX.o
INCFLAGS = 
LDFLAGS = -Wl,-rpath,/usr/local/lib
LIBS = 

all: RTX_project

RTX_project: $(OBJECTS)
	$(CC) -o RTX $(OBJECTS) $(LDFLAGS) $(LIBS)

.SUFFIXES:
.SUFFIXES:	.c .cc .C .cpp .o

.c.o :
	$(CC) -o $@ -c $(CFLAGS) $< $(INCFLAGS)

count:
	wc *.c *.cc *.C *.cpp *.h *.hpp

clean:
	rm -f *.o

.PHONY: all
.PHONY: count
.PHONY: clean
