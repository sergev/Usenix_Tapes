#
#       Makefile for MSDOS tools
#

CC = cc
CFLAGS = -O -f
LINT = lint -c
SHAR = shar -a
BINDIR = /usr/local/bin
PROGS = mdir mread mwrite mdel mtype mmd mrd mren

MDIR = mdir.o getfat.o init.o search.o match.o convdate.o unixname.o subdir.o \
	isdir.o
MREAD = mread.o getfat.o init.o search.o match.o unixname.o subdir.o
MWRITE = mwrite.o getfat.o init.o search.o fixname.o unixname.o putfat.o \
	subdir.o mkentry.o isdir.o
MDEL = mdel.o getfat.o init.o search.o match.o unixname.o putfat.o subdir.o
MTYPE = mtype.o getfat.o init.o search.o match.o unixname.o subdir.o
MMD = mmd.o getfat.o init.o search.o fixname.o unixname.o putfat.o subdir.o \
	mkentry.o
MRD = mrd.o getfat.o init.o search.o unixname.o putfat.o subdir.o
MREN = mren.o getfat.o init.o search.o fixname.o unixname.o putfat.c subdir.o \
	isdir.o

all:	$(PROGS)

mdir:	$(MDIR)
	$(CC) $(CFLAGS) $(MDIR) -o mdir

mread:	$(MREAD)
	$(CC) $(CFLAGS) $(MREAD) -o mread

mwrite:	$(MWRITE)
	$(CC) $(CFLAGS) $(MWRITE) -o mwrite

mdel:	$(MDEL)
	$(CC) $(CFLAGS) $(MDEL) -o mdel

mtype:	$(MTYPE)
	$(CC) $(CFLAGS) $(MTYPE) -o mtype

mmd:	$(MMD)
	$(CC) $(CFLAGS) $(MMD) -o mmd

mrd:	$(MRD)
	$(CC) $(CFLAGS) $(MRD) -o mrd

mren:	$(MREN)
	$(CC) $(CFLAGS) $(MREN) -o mren

install: $(PROGS)
	cp mdir $(BINDIR)/mdir
	strip $(BINDIR)/mdir
	rm mdir
#	cp Mdir.1 /usr/man/man1/mdir.1
	cp mread $(BINDIR)/mread
	strip $(BINDIR)/mread
	rm mread
#	cp Mread.1 /usr/man/man1/mread.1
	cp mwrite $(BINDIR)/mwrite
	strip $(BINDIR)/mwrite
	rm mwrite
#	cp Mwrite.1 /usr/man/man1/mwrite.1
	cp mdel $(BINDIR)/mdel
	strip $(BINDIR)/mdel
	rm mdel
#	cp Mdel.1 /usr/man/man1/mdel.1
	cp mtype $(BINDIR)/mtype
	strip $(BINDIR)/mtype
	rm mtype
#	cp Mtype.1 /usr/man/man1/mtype.1
	cp mmd $(BINDIR)/mmd
	strip $(BINDIR)/mmd
	rm mmd
#	cp Mmd.1 /usr/man/man1/mmd.1
	cp mrd $(BINDIR)/mrd
	strip $(BINDIR)/mrd
	rm mrd
#	cp Mrd.1 /usr/man/man1/mrd.1
	cp mren $(BINDIR)/mren
	strip $(BINDIR)/mren
	rm mren
#	cp Mren.1 /usr/man/man1/mren.1

lint:
	$(LINT) mdir.c getfat.c init.c search.c match.c convdate.c subdir.c \
	unixname.c isdir.c
	$(LINT) mread.c getfat.c init.c search.c match.c unixname.c subdir.c
	$(LINT) mwrite.c getfat.c init.c search.c fixname.c unixname.c \
	putfat.c subdir.c mkentry.c isdir.c
	$(LINT) mdel.c getfat.c init.c search.c match.c unixname.c putfat.c \
	subdir.c
	$(LINT) mtype.c getfat.c init.c search.c match.c unixname.c subdir.c
	$(LINT) mmd.c getfat.c init.c search.c fixname.c unixname.c putfat.c \
	subdir.c mkentry.c
	$(LINT) mrd.c getfat.c init.c search.c unixname.c putfat.c subdir.c
	$(LINT) mren.c getfat.c init.c search.c fixname.c unixname.c putfat.c \
	subdir.c

shar:
	$(SHAR) Readme Makefile Mdel.1 Mdir.1 Mmd.1 Mrd.1 Mread.1 Mren.1 \
	Mtype.1 Mwrite.1 msdos.h convdate.c fixname.c getfat.c init.c \
	isdir.c match.c mdel.c mdir.c > mtools1
	$(SHAR) mkentry.c mmd.c mrd.c mread.c mren.c mtype.c mwrite.c \
	putfat.c search.c subdir.c unixname.c > mtools2
