CFLAGS= -O -s

OBJ1= mpath.o 
OBJ2= error.o

all: makedep makenorm depend 

makedep: makedep.o $(OBJ2)
	$(CC) $(CFLAGS) $@.o $(OBJ2) -o $@

makenorm: makenorm.o $(OBJ1) $(OBJ2)
	$(CC) $(CFLAGS) $@.o $(OBJ1) $(OBJ2) -ll -o $@

depend: depend.o $(OBJ2)
	$(CC) $(CFLAGS) $@.o $(OBJ2) -ll -o $@

makenorm.o: normscan.c
depend.o: depscan.c

clean:
	rm *.o
	rm makedep
	rm makenorm makenorm.c normscan.c
	rm depend depend.c depscan.c
	rm man/*.man

.y.o:
	$(YACC) $(YFLAGS) $<
	@mv -f y.tab.c $*.c
	$(CC) $(CFLAGS) -c $*.c

.y.c:
	$(YACC) $(YFLAGS) $<
	@mv -f y.tab.c $@

.l.o:
	$(LEX) $(LFLAGS) $<
	@mv -f lex.yy.c $*.c
	$(CC) $(CFLAGS) -c $*.c

.l.c:
	$(LEX) $(LFLAGS) $<
	@mv -f lex.yy.c $@
