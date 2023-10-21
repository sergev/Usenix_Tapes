FFLAGS = -O -Nn2047
COMS = zork.com
SRCS = gdt.f init.f vappli.f zork.f
SUPD = UNIX
OBJS = gdt.o init.o vappli.o zork.o itime.o

.SUFFIXES: .r .f

.r.o:
	f77 $(FFLAGS) -c $<

.f.o:
	f77 $(FFLAGS) -c $<

zork: $(OBJS)
	f77 $(OBJS) -o zork

itime.o: $(SUPD)/itime.c
	cc -O -c $(SUPD)/itime.c

print:
	prt -jt zork 8 $(COMS) $(SRCS)
