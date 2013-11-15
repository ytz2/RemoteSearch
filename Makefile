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


LDFLAGS = -lpthread 

ifeq  ($(shell uname -s), SunOS)
  LDFLAGS += -lnsl -lsocket -lrt
endif

ifneq  ($(shell uname -s), Darwin)
  LDFLAGS += -lrt
endif

EXECUTABLES = rplcs rplcsd

all: $(EXECUTABLES)



rplcs: rplcs.o plcsIO.o command_util.o str_search.o rpath.o ospenv.h \
	dirHandle.o client.o no_sigpipe.o tcpblockio.o send_recv.o \
	search_given.o  thread_share.o print_time.o
	$(CC) $(CFLAGS) $(LDFLAGS)  rplcs.o plcsIO.o command_util.o rpath.o \
	str_search.o dirHandle.o client.o no_sigpipe.o tcpblockio.o send_recv.o \
	search_given.o thread_share.o print_time.o -o rplcs

rplcsd: server.o rplcsd.o tcpblockio.o no_sigpipe.o \
		send_recv.o plcsIO.o str_search.o rpath.o dirHandle.o \
		search_given.o thread_share.o print_time.o
	$(CC) $(CFLAGS) $(LDFLAGS) server.o rplcsd.o tcpblockio.o \
	no_sigpipe.o command_util.o send_recv.o plcsIO.o str_search.o \
	rpath.o dirhandle.o search_given.o thread_share.o print_time.o -o rplcsd 

dirHandle.o:  ospenv.h rpath.h dirHandle.h thread_share.h
	$(CC) $(CFLAGS)  -c dirHandle.c

rplcs.o: rplcs.c  str_search.h command_util.h ospenv.h print_time.h
	$(CC) $(CFLAGS)  -c rplcs.c

rplcsd.o: rplcsd.c server.h
	$(CC) $(CFLAGS)  -c rplcsd.c
	
client.o:	client.c client.h no_sigpipe.h tcpblockio.h \
send_recv.h command_util.h thread_share.h
	$(CC)	$(CFLAGS) -c client.c

server.o:   server.c server.h no_sigpipe.h tcpblockio.h send_recv.h print_time.h \
 command_util.h plcsIO.h search_given.h thread_share.h
	$(CC)	$(CFLAGS) -c server.c
	
plcsIO.o: plcsIO.h plcsIO.c str_search.h ospenv.h client.h send_recv.h thread_share.h
	$(CC) $(CFLAGS)   -c plcsIO.c	

command_util.o: command_util.h command_util.c ospenv.h
	$(CC) $(CFLAGS) -c command_util.c

rpath.o: rpath.h rpath.c 
	$(CC) $(CFLAGS) -c rpath.c

str_search.o:  str_search.h str_search.c ospenv.h rpath.h 
	$(CC) $(CFLAGS) -c  str_search.c
	
send_recv.o:	send_recv.c send_recv.h 
	$(CC)	$(CFLAGS) -c send_recv.c

tcpblockio.o:	tcpblockio.c tcpblockio.h
	$(CC)	$(CFLAGS) -c tcpblockio.c

no_sigpipe.o:	no_sigpipe.c no_sigpipe.h
	$(CC)	$(CFLAGS) -c no_sigpipe.c
	
search_given.o: search_given.h search_given.c ospenv.h command_util.h \
	str_search.h rpath.h dirHandle.h \
	plcsIO.h client.h
	$(CC)	$(CFLAGS) -c search_given.c
thread_share.o: thread_share.h thread_share.c global.h
	$(CC)	$(CFLAGS) -c thread_share.c
print_time.o: print_time.h print_time.c
	$(CC)	$(CFLAGS) -c print_time.c
clean:
	rm -rf *.o *~ $(EXECUTABLES)
