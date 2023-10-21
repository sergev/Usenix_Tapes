#  Makefile for the demo rpc service for rpcgen
#
#  There are three source files used by this makefile:
#	demo_xdr.x	Demo protocol specification used as input
#			by rpcgen to generate demo_xdr.h (xdr
#			defines and data type decarations),
#			demo_xdr.c (xdr data type routines),
#			and demo.c (server program shell)
#	demo_proc.c	The actual procedure handlers for the server.
#	demo_clnt.c	The client program.

.SUFFIXES: .x .c .h .o
CC = cc

# REL3_0 should be defined in CFLAGS if using a pre-3.2 rpc library
CFLAGS = -DREL3_0

RPCGEN = ../rpcgen
PROTO = udp

SOBJS = demo.o demo_proc.o demo_xdr.o 
COBJS = demo_clnt.o demo_xdr.o 

# RPCLIB should be the rpc library, or commented out if included in libc.
# xdr_update.o should be included if linking with a pre-3.2 rpc library.
#RPCLIB = -lrpc
RPCLIB = ../xdr_update.o

.x.c:
	$(RPCGEN) -c $< -o $@
.x.h:
	$(RPCGEN) -h $< -o $@

all: demo_svr demo_clnt

demo_svr: $(SOBJS)
	$(CC) $(CFLAGS) -o $@ $(SOBJS) $(RPCLIB)

demo_clnt: $(COBJS)
	$(CC) $(CFLAGS) -o $@ $(COBJS) $(RPCLIB)

demo.c: demo_xdr.x demo_xdr.h
	$(RPCGEN) -s $(PROTO) demo_xdr.x -o $@

demo_xdr.c: demo_xdr.x
demo_xdr.o: demo_xdr.c demo_xdr.h
demo_proc.o: demo_xdr.h
demo_clnt.o: demo_xdr.h

clean:
	-rm -f *.o *.h demo_clnt demo_svr demo_xdr.c demo.c
