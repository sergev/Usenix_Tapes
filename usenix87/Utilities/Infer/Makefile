CFLAGS = -O
LDFLAGS = -n
OBJECTS = infer.o compile.o
SOURCES = infer.c compile.c
BIN = /u/danny/bin

infer: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o infer

$(OBJECTS): infer.h

clean:
	rm -f $(OBJECTS) core

clobber: clean
	rm -f infer

install: infer
	cp infer $(BIN)/infer
	strip $(BIN)/infer

lint:
	lint $(SOURCES) >fluff
