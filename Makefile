# Created by Yanhua Liu
# Makefile for CS820 Assignment 2
# Object: plcs


CC=gcc

UNAME_S := $(shell uname -s)

# CFLAGS
ifeq    ($(UNAME_S),SunOS)
	CFLAGS= -Wall -O -g
else
	CFLAGS=  -Wall -O -g -Wextra
endif


# PFLAGS
ifeq    ($(UNAME_S),Darwin)
	PFLAGS  = 
endif

ifeq    ($(UNAME_S),SunOS)
	PFLAGS  = 
endif

ifeq      ($(UNAME_S),Linux)
	PFLAGS  = -lpthread
endif


all: plcs.o plcsIO.o command_util.o str_search.o rpath.o ospenv.h dirHandle.o
	$(CC) $(CFLAGS) $(PFLAGS)  plcs.o plcsIO.o command_util.o rpath.o str_search.o dirHandle.o -o plcs

dirHandle.o:  ospenv.h rpath.h dirHandle.h global.h
	$(CC) $(CFLAGS) $(PFLAGS) -c dirHandle.c

plcs.o: plcs.c global.h  str_search.h command_util.h ospenv.h
	$(CC) $(CFLAGS) $(PFLAGS) -c plcs.c

plcsIO.o: plcsIO.h plcsIO.c  global.h str_search.h ospenv.h
	$(CC) $(CFLAGS) $(PFLAGS)  -c plcsIO.c	

command_util.o: command_util.h command_util.c ospenv.h
	$(CC) $(CFLAGS) -c command_util.c

rpath.o: rpath.h rpath.c 
	$(CC) $(CFLAGS) -c rpath.c

str_search.o:  str_search.h str_search.c ospenv.h rpath.h
	$(CC) $(CFLAGS) -c  str_search.c

clean:
	rm -rf *.o *~ plcs
