CFILES = cmdbymail.c
OFILES = cmdbymail.o
CFLAGS = -O

all: cmdbymail cmdbymail.nr

cmdbymail: $(OFILES)
        cc -o $(OFILES) cmdbymail

cmdbymail.nr: cmdbymail.1
        nroff -Tla12 -man cmdbymail.1 > cmdbymail.nr

