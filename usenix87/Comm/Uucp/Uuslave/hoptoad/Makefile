# Fix this sometime soon!

DEFS =	-DBSD -DDEBUG -DSUBDIR -DLOG
SHAR1 =	README ATT.CERTIFICATION PORTING BUGS Makefile uuslave.c
SHAR2 =	packet.driver.ms comport.h comport.asm

uuslave: uuslave.c
	cc -o uuslave -g uuslave.c $(DEFS)

lint:
	lint -hbxn $(DEFS) uuslave.c

shar:
	shar $(SHAR1) >uuslave.shar.1
	shar $(SHAR2) >uuslave.shar.2
