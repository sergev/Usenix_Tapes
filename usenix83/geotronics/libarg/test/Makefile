#
all:    getopt  grep

getopt: getopt.o
	$(CC) -o getopt getopt.o -larg

grep:   grep.o
	$(CC) -o grep grep.o -larg

clean:
	rm -f *.o
	rm -f getopt grep
	rm -f z*
