OBJS = main.obj comp.obj getpath.obj mname.obj setps.obj curdrv.obj \
first.obj getsize.obj next.obj shortlis.obj curpath.obj gcdate.obj \
longlist.obj putname.obj toscreen.obj endlin.obj getcl.obj search.obj 

ls.exe : $(OBJS)
	link $(OBJS),ls.exe;

$(OBJS) : intdefs.h $*.c
	cl -c -DMSC $*.c


