# Makefile for less taken from usenet and modified for msdos.
# For use with microsoft c. (4.0 used but 3.0 will work just fine)
# This makefile has been kept simple so that just about any make will do.
# I use the make posted by Neil Russell. If you want a port of it to MSDOS
#  contact me. Address given in readme. It is virtually Unix compatable.
OBJS1 = main.obj ch.obj command.obj help.obj input.obj line.obj option.obj
OBJS2 = option.obj output.obj position.obj prim.obj prompt.obj
OBJS3 = signal.obj ttyin.obj version.obj
OBJS4 = regexp.obj regerror.obj scrn.obj
less.exe : $(OBJS1) $(OBJS2) $(OBJS3) $(OBJS4)
	link @files
main.obj : main.c less.h position.h
	msc -DDOSPORT main;
option.obj : option.c less.h
	msc -DDOSPORT option;
prim.obj : prim.c less.h position.h
	msc -DDOSPORT prim;
ch.obj : ch.c less.h
	msc -DDOSPORT ch;
position.obj : position.c less.h position.h
	msc -DDOSPORT position;
input.obj : input.c less.h
	msc -DDOSPORT input;
output.obj : output.c less.h scrn.h
	msc -DDOSPORT output;
prompt.obj : prompt.c less.h position.h
	msc -DDOSPORT prompt;
line.obj : line.c less.h
	msc -DDOSPORT line;
signal.obj : signal.c less.h
	msc -DDOSPORT signal;
help.obj : help.c less.h
	msc -DDOSPORT help;
ttyin.obj : ttyin.c less.h
	msc -DDOSPORT ttyin;
command.obj : command.c less.h position.h
	msc -DDOSPORT command;
version.obj : version.c
	msc -DDOSPORT version;
regexp.obj : regexp.c regexp.h regmagic.h
	msc regexp;
regerror.obj : regerror.c
	msc -DDOSPORT regerror;
scrn.obj : scrn.c
	msc scrn;

