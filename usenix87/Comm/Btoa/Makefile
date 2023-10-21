CC=cc -O -s
BIN=/usr/local/bin
MAN=/usr/man/manl
L=l

install: atob btoa tarmail untarmail
	rm -f $(BIN)/atob $(BIN)/btoa $(BIN)/tarmail $(BIN)/untarmail
	mv atob btoa $(BIN)
	cp tarmail untarmail $(BIN)
	make clean

man:	btoa.man
	rm -f $(MAN)/btoa.$(L) $(MAN)/tarmail.$(L)
	cp btoa.man $(MAN)/btoa.$(L)
	cp btoa.man $(MAN)/tarmail.$(L)
	echo Now, run catman.

atob:	atob.c
	$(CC) atob.c -o atob

btoa:	btoa.c
	$(CC) btoa.c -o btoa

clean:
	rm -f *.o atob btoa
