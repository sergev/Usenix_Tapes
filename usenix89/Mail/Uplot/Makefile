#
# Makefile for compiling uplot on a DEC Rainbow running VENIX/86
# For other machines you will likely want LDFLAGS = -lplot, or something
#

CFLAGS = -z -i
LDFLAGS= -ltrb
SHARFILES = README uplot.c uplot.sed uplot.sh Makefile

uplot: uplot.c
	cc -o uplot $(CFLAGS) uplot.c $(LDFLAGS)

uplot.shar: $(SHARFILES)
	shar $(SHARFILES) >uplot.shar
