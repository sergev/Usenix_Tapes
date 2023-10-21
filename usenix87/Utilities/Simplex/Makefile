BINARY = simplex

SOURCES = main.c f.c sum_residual.c enter.c first.c new_vertex.c order.c report.c

OBJECTS = main.o f.o sum_residual.o enter.o first.o new_vertex.o order.o report.o

LIBRARIES = -lm -lc

CFLAGS = -O
LDFLAGS = -n -s -x
LINTFLAGS =

$(BINARY): $(OBJECTS)
	@echo "	loading $(BINARY)"
	@ld -o  $(LDFLAGS) /lib/crt0.o $(OBJECTS) $(LIBRARIES)

lint:
	lint $(LINTFLAGS) $(SOURCES)

$(OBJECTS): simplex.h
