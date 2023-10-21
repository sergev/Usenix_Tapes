LOG = /usr/local/lib/snake.scores

# choose system type from one of EUNICE, BSD42, BSD41, SYSIII, SYSV
UNIX = BSD42

# choose timeout mechanism from one of BSD_SELECT, CMU_IPC, VMS
TIMER = BSD_SELECT

CFLAGS = -O -D$(UNIX) -D$(TIMER) -DLOG=\"$(LOG)\"

LIBS = -lcurses -ltermcap

snake2: snake2.o quicksleep.o getkey.o
	cc -o snake2 $(CFLAGS) snake2.o quicksleep.o getkey.o -lcurses -ltermcap

install:
	strip snake2
	cat /dev/null >> $(LOG)
	chmod 644 $(LOG)
	/etc/chown daemon snake2 $(LOG)
	chmod 4755 snake2
	mv snake2 /usr/local/bin

snake2.o: snake2.h
