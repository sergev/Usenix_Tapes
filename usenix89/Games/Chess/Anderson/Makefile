#
# Makefile for nchess package.
#

ALLOBJS = board.o boardsw.o main.o ipc.o controlsw.o daemon.o \
	tool.o xdr.o msgsw.o talksw.o 
CLIENTOBJS = board.o main.o ipc.o xdr.o tool.o controlsw.o boardsw.o \
	msgsw.o talksw.o chessprocess.o
SERVEROBJS = daemon.o xdr.o

CLIENTSRCS1 = board.c main.c 
CLIENTSRCS2 = ipc.c tool.c controlsw.c 
CLIENTSRCS3 = boardsw.c msgsw.c talksw.c chessprocess.c
CLIENTSRCS4 = xdr.c
CLIENTSRCS = $(CLIENTSRCS1) $(CLIENTSRCS2) $(CLIENTSRCS3) $(CLIENTSRCS4)

SERVERSRCS = daemon.c xdr.c

HEADERS = decls.h nchess.h

ICONS = Icons/bishop.icon Icons/bishopStencil.icon \
	Icons/king.icon Icons/kingStencil.icon \
	Icons/knight.icon Icons/knightStencil.icon \
	Icons/pawn.icon Icons/pawnStencil.icon \
	Icons/queen.icon Icons/queenStencil.icon \
	Icons/rook.icon Icons/rookStencil.icon \
	Icons/blackSquare.icon Icons/whiteSquare.icon \
	Icons/nchess.icon

# CFLAGS = -g
CFLAGS = -O

all : nchessd nchess

nchessd : $(SERVEROBJS)
	cc $(CFLAGS) -o nchessd $(SERVEROBJS)

nchess : $(CLIENTOBJS)
	cc $(CFLAGS) -o nchess $(CLIENTOBJS) -lsuntool -lsunwindow -lpixrect

shar : $(SERVERSRCS) $(CLIENTSRCS) $(HEADERS) $(ICONS)
	shar -c Makefile nchess.6 $(HEADERS) $(SERVERSRCS) Icons $(ICONS) > shar.1
	shar -c $(CLIENTSRCS1) > shar.2
	shar -c $(CLIENTSRCS2) > shar.3
	shar -c $(CLIENTSRCS3) > shar.4

lint : $(SERVERSRCS) $(CLIENTSRCS)
	lint $(SERVERSRCS)
	lint $(CLIENTSRCS)

list : $(SERVERSRCS) $(CLIENTSRCS) $(HEADERS)
	lpr -x2 -p -J"nchess" *.h *.c

clean : 
	rm -f *.o nchess nchessd

$(ALLOBJS) : $(HEADERS)

