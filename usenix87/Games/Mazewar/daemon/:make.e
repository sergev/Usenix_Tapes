	cc -O -DPLUS5 curtime.o getsocket.o init.o join.o main.o send.o service.o  syserr.o ualarm.o verify.o work.o vers.o -o mazed
undefined	first referenced
 symbol  	    in file
_socket 	getsocket.o
_bzero  	getsocket.o
_bind   	getsocket.o
_dup2   	init.o
_bcopy  	join.o
_sendto 	send.o
_setitim	ualarm.o
_inet_nt	verify.o
_gethost	verify.o
_recvfro	work.o
_sigbloc	work.o
_sigsetm	work.o
ld fatal: Symbol referencing errors. No output written to mazed
*** Error code 13

Stop.
