#
#  Makefile for dpr, Sun general purpose screen dump utility.
#  Copyright  (c) Rich Burridge, Sun Australia 1986.
#
#  Version 1.1.
#
#  No responsibility is taken for any errors inherent either in the comments
#  or the code of this program, but if reported to me then an attempt will
#  be made to fix them.
#
BINARIES      = dpr testpw_c testpw_f testpw_p
BINDIR        = ./release
CCFLAGS       = -c -O
PCFLAGS       = -c -O
FFLAGS        = -c -O
LDFLAGS       =
OBJS          = dpr_dumpcap.o dpr_filter.o dpr_sun.o
SRCS          = dpr_main.c dpr_dumpcap.c dpr_filter.c dpr_sun.c dpr_pw.c
LIBS          = -lsunwindow -lpixrect
HDRS          = dpr.h globals.h
OTHERS        = testpw_c.c testpw_f.f testpw_p.p
#
#  The following commands are declared:
#
all:           $(BINARIES)
		echo -n "" >/dev/tty
release:       $(BINARIES)
		ar q libdpr.a $(OBJS) dpr_pw.o
		mv libdpr.a $(BINDIR)
		ranlib $(BINDIR)/libdpr.a
		rm -f *.o core
		strip $(BINARIES)
		mv dpr $(BINDIR)
		mv testpw_c $(BINDIR)
		mv testpw_f $(BINDIR)
		mv testpw_p $(BINDIR)
		cp testpw_c.c $(BINDIR)
		cp testpw_f.f $(BINDIR)
		cp testpw_p.p $(BINDIR)
		cp dumpcap $(BINDIR)
		cp dpr.1 $(BINDIR)
		cp dumpcap.5 $(BINDIR)
#
#  General Makefile stuff.
#
dpr:            dpr_main.o $(OBJS)
		cc $(LDFLAGS) -o dpr dpr_main.o $(OBJS) $(LIBS)
dpr_main.o:     globals.h dpr.h
		cc $(CCFLAGS) dpr_main.c
dpr_dumpcap.o:  dpr.h
		cc $(CCFLAGS) dpr_dumpcap.c
dpr_filter.o:   globals.h
		cc $(CCFLAGS) dpr_filter.c
dpr_pw.o:       globals.h dpr.h
		cc $(CCFLAGS) dpr_pw.c
dpr_sun.o:      dpr.h
		cc $(CCFLAGS) dpr_sun.c
testpw_c.o:
		cc $(CCFLAGS) testpw_c.c
testpw_c:       testpw_c.o dpr_pw.o $(OBJS)
		cc $(LDFLAGS) -o testpw_c testpw_c.o dpr_pw.o dpr_dumpcap.o $(LIBS)
testpw_f.o:
		f77 $(FFLAGS) testpw_f.f
testpw_f:       testpw_f.o dpr_pw.o $(OBJS)
		f77 $(LDFLAGS) -o testpw_f testpw_f.o dpr_pw.o dpr_dumpcap.o $(LIBS)
testpw_p.o:
		pc $(PCFLAGS) testpw_p.p
testpw_p:       testpw_p.o dpr_pw.o $(OBJS)
		pc $(LDFLAGS) -o testpw_p testpw_p.o dpr_pw.o dpr_dumpcap.o $(LIBS)
#
#  Miscellaneous useful pseudo-makes.
#
backup:
		tar cvbf 126 /dev/rst0 .
clean:
		rm -f *.o core
		strip $(BINARIES)

lint:
	lint $(SRCS) /usr/lib/lint/llib-lpixrect.ln
