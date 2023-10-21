CFLAGS = -O -q
PATH = .
LIBP = -lP

all:    ff init new update fix

ff:     ff.o ffglb.o $(PATH)/fflib.a
	$(CC) -n -q ff.o ffglb.o $(PATH)/fflib.a $(LIBP) -lS -o ff

ff.o:   ff.c $(PATH)/ffdef.h
	$(CC) $(CFLAGS) -c ff.c

init:   init.o ffglb.o $(PATH)/fflib.a
	$(CC) -q init.o ffglb.o $(PATH)/fflib.a $(LIBP) -lS -o init

init.o: init.c $(PATH)/ffdef.h
	$(CC) $(CFLAGS) -c init.c

new:    new.o ffglb.o $(PATH)/fflib.a
	$(CC) -q new.o ffglb.o $(PATH)/fflib.a $(LIBP) -lS -o new

new.o:  new.c $(PATH)/ffdef.h
	$(CC) $(CFLAGS) -c new.c

update: update.o ffglb.o $(PATH)/fflib.a
	$(CC) -n -q update.o ffglb.o $(PATH)/fflib.a $(LIBP) -lS -o update

update.o: update.c $(PATH)/ffdef.h
	$(CC) $(CFLAGS) -c update.c

fix: fix.o ffglb.o $(PATH)/fflib.a
	$(CC) -q fix.o ffglb.o $(PATH)/fflib.a $(LIBP) -lS -o fix

fix.o: fix.c $(PATH)/ffdef.h
	$(CC) $(CFLAGS) -c fix.c

ffglb.o:  ffglb.c $(PATH)/ffdef.h
	$(CC) $(CFLAGS) -c ffglb.c

$(PATH)/fflib.a: $(PATH)/SUBS

$(PATH)/SUBS:
	@echo "Run make in $(PATH)/SUBS first"
