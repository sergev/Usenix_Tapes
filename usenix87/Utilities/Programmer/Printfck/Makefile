# Makefile for printfck
#
# Change printfck.h so that PERCENT_HEADERS and PERCENT_ROUTINES
# have the correct directory paths.


all: printfck percent.h


printfck: printfck.o printfck.h
        cc -o printfck printfck.o

percent.h: percent.c
        sed -e "s/LINTLIBRARY//" -e "s/(x.*/();/" percent.c >percent.h
