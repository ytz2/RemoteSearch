# Created by Yanhua Liu
# Makefile for CS820 Assignment 1
# Object: plcs


CC=gcc


ifeq ($ (OSTYPE),solaris)
	CFLAGS= -Wall -O -g
else
	CFLAGS= -O -Wall  -g -Wextra
endif


all: plcs.o plcsIO.o command_util.o str_search.o ospenv.h
	$(CC) $(CFLAGS)  plcs.o plcsIO.o command_util.o str_search.o -o plcs

plcs.o: plcs.c global.h  str_search.h command_util.h ospenv.h
	$(CC) $(CFLAGS) -c plcs.c

plcsIO.o: plcsIO.h plcsIO.c  global.h str_search.h ospenv.h
	$(CC) $(CFLAGS) -c plcsIO.c	

command_util.o: command_util.h command_util.c ospenv.h
	$(CC) $(CFLAGS) -c command_util.c

str_search.o:  str_search.h str_search.c ospenv.h
	$(CC) $(CFLAGS) -c str_search.c

clean:
	rm -rf *.o *~