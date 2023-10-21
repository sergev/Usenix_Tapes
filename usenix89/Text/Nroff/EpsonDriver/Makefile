CFLAGS = -O
LDFLAGS = -s
epx:	epx.o nlq.o draft.o
	$(CC) $(LDFLAGS) epx.o nlq.o draft.o -o epx
test:
	neqn special | nroff -Tepson | col -fx | epx | lp -dEpson_R
lint:
	lint epx.c nlq.c draft.c


