cdecl : cdgram.o cdsupp.o
	cc -o cdecl cdgram.o cdsupp.o

cdgram.o : cdgram.c cdlex.c

cdlex.c : cdlex.l
	lex cdlex.l
	mv lex.yy.c cdlex.c

cdgram.c : cdgram.y
	yacc cdgram.y
	mv y.tab.c cdgram.c

clean:
	rm -f cdecl cdgram.o cdsupp.o cdlex.c cdgram.c
