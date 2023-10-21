OBJS = hash.o node.o parse.o y.tab.o
MOBJS = hash.o node.o y.tab.o

all:	parse path

node:	node.o
		cc -c node.c
path:	maketree.o $(MOBJS)
	cc -o path maketree.o $(MOBJS)

	chmod 0711 path
parse:	$(OBJS)
	cc -o parse $(OBJS)
	chmod 0711 parse

y.tab.c:	exp.y
	yacc exp.y
