kbq:	kb.o kbq.o fsindex.o sindex.o dir.o
	cc -o kbq kb.o fsindex.o dir.o kbq.o
	chmod +x kbq
	strip kbq
	ln kbq kbu
	ln kbq kbc
	ln kbq kbk
	ln kbq kbx

kbq.o:	kbq.c kb.h
	cc -O -c kbq.c

kb.o:	kbq.c kb.h
	cc -O -c kb.c

