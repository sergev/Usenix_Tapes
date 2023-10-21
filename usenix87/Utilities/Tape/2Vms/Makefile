CFILES	=	field.c	header3.c	skiptm.c	vmstape.c	vt_append.c \
	vt_extract.c	vt_list.c	vt_write.c

OBJECTS =	field.o	header3.o	skiptm.o	vmstape.o	vt_append.o \
	vt_extract.o	vt_list.o	vt_write.o

CMD	= vmstape
DESTIN	= /usr/local/bin
CFLAGS	= -O $(DEFS)
DEFS	= -DNEWDIR
# to compile for ver 4.2 BSD  OR any version that CANNOT emulate 
#		the 4.2 directory structure with the files ndir.h and libndir.a:
#	define in DEFS '-Dnewdir'.
#	do not define '-lndir' in LIBS.
# to compile for any version of unix WITH the 4.2 directory EMULATION package:
# 	do not define  '-Dnewdir' in DEFS.
#	define '-lndir' in LIBS.
LIBS	= 
OWNER	= 
MODE	= 0755

$(CMD)	: $(OBJECTS) /lib/libc.a 
	ld /lib/crt0.o $(OBJECTS) -o $(CMD) $(LIBS) -lc -lg

install	: $(DESTIN)/$(CMD)

$(DESTIN)/$(CMD) : $(CMD)
	cp $(CMD) $(DESTIN)/$(CMD)
	strip $(DESTIN)/$(CMD)
	chmod $(MODE) $(DESTIN)/$(CMD)

depend	:
	@makedepend $(CFILES)

clean	:
	-rm $(OBJECTS) $(CMD) 

print	:
	print -h $(CFILES) 

lint	:
	lint -h $(CFILES) $(DEFS)

# DO NOT DELETE THIS LINE -- make depends on it

header3.o	: vmstape.h

skiptm.o	: /usr/include/sys/types.h /usr/include/sys/mtio.h \
	 vmstape.h /usr/include/sys/ioctl.h

vmstape.o	: /usr/include/stdio.h vmstape.h

vt_append.o	: /usr/include/stdio.h /usr/include/sys/types.h \
	 /usr/include/sys/stat.h /usr/include/sys/mtio.h vmstape.h \
	/usr/include/sys/ioctl.h

vt_extract.o	: /usr/include/stdio.h /usr/include/ctype.h \
	 /usr/include/sys/types.h /usr/include/sys/stat.h vmstape.h

vt_list.o	: vmstape.h

vt_write.o	: /usr/include/stdio.h /usr/include/sys/types.h \
	 /usr/include/sys/stat.h /usr/include/sys/dir.h vmstape.h
