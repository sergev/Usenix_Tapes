all: cut paste


paste: paste.o spaste.o getopt.o
	cc -o paste paste.o spaste.o getopt.o

cut: cut.o getopt.o
	cc -o cut cut.o getopt.o

lint: lint_cut lint_paste

lint_cut:
	lint cut.c getopt.c

lint_paste:
	lint paste.c spaste.c getopt.c

