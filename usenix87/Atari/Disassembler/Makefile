OBJS = main.o initopts.o lex.o ref.o print.o tbl.o 
SRCS = dis.h main.c initopts.c lex.l ref.c print.c tbl.c 
CFLAGS = -O

dis:		$(OBJS)
		cc $(OBJS) -o dis

tbl.o:		dis.h tbl.c
		cc -c tbl.c

initopts.o:	dis.h initopts.c

main.o:		dis.h main.c

lex.o:		dis.h lex.l

ref.o:		dis.h ref.c

print.o:	dis.h print.c

dis.man:	dis.1
		nroff -man dis.1 > dis.man

install:	dis
		cp dis /a/rgb/bin/dis6502

clean:
		rm -f $(OBJS)

clobber:	clean
		rm -f dis

ckpt:		$(SRCS)
		ci -l $(SRCS)

shar:		Makefile dis.1 dis.man $(SRCS)
		shar -a dis.man Makefile dis.1 $(SRCS) > dis.shar
