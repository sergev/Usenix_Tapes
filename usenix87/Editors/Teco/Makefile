##
##  TECO MAKEFILE
##

# Debugging or optimizing?
#CFLAGS	= -g
CFLAGS	= -O
TERMCAP	= -ltermcap

OBJECTS	= te_data.o te_exec0.o te_exec1.o te_exec2.o te_main.o te_rdcmd.o \
	te_srch.o te_subs.o te_utils.o te_window.o

te:		$(OBJECTS) te_chario.o
	$(CC) -o te $(CFLAGS) $(OBJECTS) te_chario.o $(TERMCAP)

tex:		$(OBJECTS) te_chx.o
	$(CC) -o tex $(CFLAGS) $(OBJECTS) te_chx.o $(TERMCAP)

$(OBJECTS) te_chario.o te_chx.o:	te_defs.h
te_chx.o:				te_chario.c
	cp te_chario.c te_chx.c
	$(CC) -DDEBUG $(CFLAGS) -c te_chx.c

