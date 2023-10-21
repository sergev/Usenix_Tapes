	cc -O -DPLUS5 curtime.o getsocket.o init.o join.o main.o send.o service.o  syserr.o ualarm.o verify.o work.o vers.o -o mazed
undefined		first referenced
 symbol  		    in file
_socket             		getsocket.o
_bzero              		getsocket.o
_bind               		getsocket.o
_dup2               		init.o
_bcopy              		join.o
_sendto             		send.o
_setitimer          		ualarm.o
_inet_ntoa          		verify.o
_gethostbyaddr      		verify.o
_recvfrom           		work.o
_sigblock           		work.o
_sigsetmask         		work.o
ld fatal: Symbol referencing errors. No output written to mazed
*** Error code 13

Stop.
