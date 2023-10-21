MAKEFILE = Makefile
TARGET	= multivol

#	USER - user name used for sendfile requests
#	HOSTS - sendfile hosts
USER	= tony
HOSTS	= pisa rune uq ppse madvax1

#	installation library
INSTALL = /user0/tony/Bin

#	MISC - misc. files to be listed and sent to other hosts
MISC	= multivol.8 testdata.c READ_ME maint.doc
HEADERS	= multivol.h options.h trace.h
SRC1	= multivol.c rd.c wr.c
SRC2	= vhdr.c mount.c bcopy.c ask.c error.c trace.c
SOURCES	= $(SRC1) $(SRC2)
OBJECTS	= multivol.o rd.o wr.o vhdr.o mount.o ask.o error.o trace.o
SHSRC	=

#	used to compile in bcopy() if it is not defined on the system
LIBRARY	= multivol.a
LIBSRC	= bcopy.c
LIBOBJ	= bcopy.o

#	**** shar switches ****
SHAR	= shar -c -b -v

#	**** defaults for compiled code ****
#	DEFDEV - default device
#	DEFLIM - default block limit (-1 = no limit)
#	DEFBSIZ - default block size
DEFDEV	= /dev/multivol
DEFBSIZ	= 4096
DEFBLIM	= -1

#	**** compiler, loader & lint switches ****
#	DEBUG 	- compile in traceing code.
#	VOLNUM	- read/write to <device>.<vol-number> to test multiple volumes
#		  with files pretending to be device volumes.
#	REWIND	- manually rewind tape after reading volume header
DEFINE	= -DDEBUG -Dvoid=int
#DEFINE	= -Dvoid=int
LINT	= -p $(DEFINE) 
#CFLAGS	= -O $(DEFINE)
CFLAGS	= $(DEFINE)
LDFLAGS	= -n

#	**** Variables used for Testing ****
#	TSTFIL - Test file pretending to be a device - see wfile: and rfile:
#	TSTDEV - Test device
TSTFIL	= vol
#	test device
#TSTDEV	= $(TSTFIL)
TSTDEV	= $(DEFDEV)
#TSTDEV	= /dev/flop
#TSTDEV	= /dev/rx2
#TSTDEV	= /dev/rmt0
#TSTDEV	= /dev/fd70

#	Testing utilities use these switches :-
#	These can be easily individually changed when invoking make
#	LABEL	- volume label
#	BLKSIZ	- Block size used in test runs
#	BLKLIM	- Limit of blocks per volume ( -1 = no limit )
#	OPT	- other multivol options
LABEL	= -l testdata_label
BLKSIZ	= -b $(DEFBSIZ)
BLKLIM	= -n 10
#OPT	= -vw
OPT	= -tD
RDOPTS	= -i $(OPT) $(BLKLIM) $(LABEL)
WROPTS	= -o $(OPT) $(BLKSIZ) $(BLKLIM) $(LABEL)

#	OUTDTA	- temporary file for comparing with output from 'testdata'.
#	TSTSH	- temporary file for storing the last testdata command 
OUTDTA	= $(TARGET).out
TSTSH	= $(TARGET).sh

$(TARGET):	$(OBJECTS) $(LIBRARY)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS) -lc $(LIBRARY)
# for uq PDP11. to use correct ctime() routines.
#	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS) -lstd -lc $(LIBRARY)
#	size $@

multivol.o:		multivol.c
	$(CC) $(CFLAGS) -DDEFBSIZ=$(DEFBSIZ) -DDEFBLIM=$(DEFBLIM) \
			-DDEFDEV='"$(DEFDEV)"' -c multivol.c

$(LIBRARY):	$(LIBOBJ)
	ar ru $(LIBRARY) $(LIBOBJ)
#	ranlib $(LIBRARY)

testdata:	testdata.o
	$(CC) $(LDFLAGS) -o testdata testdata.o

#	******* Testing utilities for multivol *******
odfile:
	od -c $(TSTFIL) | less

od:
	od -c $(TSTDEV) | less

# Test displaying volume header of a file
tfile:	$(TARGET) 
	./$(TARGET) -t $(TSTFIL)

# Test writing of files pretending to be devices - quick test
wfile:	testdata $(TARGET) # write data out to file
	>$(TSTFIL)
	echo './testdata 1005 xyz 1022 abc' > $(TSTSH)
	sh $(TSTSH) | ./$(TARGET) $(WROPTS) $(TSTFIL)

# Test reading files pretending to be devices
# Read file volume and compare output with the last input data written
rfile: 	$(TARGET)
	./$(TARGET) $(RDOPTS) $(TSTFIL) > $(OUTDTA)
	sh $(TSTSH) | cmp - $(OUTDTA)

# Test re-writing of files pretending to be devices
rewfile:	testdata $(TARGET) wfile
	echo './testdata 4096 zyxwvutsrqponml' > $(TSTSH)
	sh $(TSTSH) |./$(TARGET) $(WROPTS) $(TSTFIL)

# Test displaying volume header
t:	$(TARGET) 
	./$(TARGET) -t $(TSTDEV)

# Read volume and compare output with the last input data written
r: 	$(TARGET)
	./$(TARGET) $(RDOPTS) $(TSTDEV) > $(OUTDTA)
	sh $(TSTSH) | cmp - $(OUTDTA)

# Read volume and count wo
ro: 	$(TARGET)
	./$(TARGET) $(RDOPTS) $(TSTDEV)

# Test writing of volumes - quick test
w:	testdata $(TARGET) # write data out to device
	echo './testdata 30 abcdefghi' > $(TSTSH) 
	sh $(TSTSH) | ./$(TARGET) $(WROPTS) $(TSTDEV)

# Test writing off end of a volume (NCR floppy)
wpisa:	testdata $(TARGET) # write data out to device
	echo './testdata 7680 abcdefghijklmno' > $(TSTSH) 
	sh $(TSTSH) | ./$(TARGET) $(WROPTS) $(TSTDEV)

# Test writing off end of a volume (NCR floppy)
wwords:	testdata $(TARGET) # write data out to device
	echo 'cat /usr/dict/words' > $(TSTSH) 
	sh $(TSTSH) | ./$(TARGET) $(WROPTS) $(TSTDEV)

# Read volume and compare output with the last input data written
rwords: 	$(TARGET)
	./$(TARGET) $(RDOPTS) $(TSTDEV) | diff - /usr/dict/words

# Test writing off end of a volume (rune host, 1600 bpi tape)
#	100 blocks = 4088 * 100 - 68 = 102183 * 4  (4 chars = 'abc\n')
wrune:	testdata $(TARGET) # write data out to device
	echo './testdata 102183 abc' > $(TSTSH) 
	sh $(TSTSH) | ./$(TARGET) $(WROPTS) -n -1 $(TSTDEV)

# Test writing 2 volume set
w2vol:	testdata $(TARGET) # write data out to device
	echo './testdata 7000 abcdefghi' > $(TSTSH) 
	sh $(TSTSH) | ./$(TARGET) $(WROPTS) $(TSTDEV)

# Test writing 3 volume set
w3vol:	testdata $(TARGET) # write data out to device
	echo './testdata 12000 abcdefghi' > $(TSTSH) 
	sh $(TSTSH) | ./$(TARGET) $(WROPTS) $(TSTDEV)

# Test writing off end of a volume
wbig:	testdata $(TARGET) # write data out to device
	echo './testdata 100000 abcdefghijklmno' > $(TSTSH) 
	sh $(TSTSH) | ./$(TARGET) $(WROPTS) $(TSTDEV)

#	******* General utilities *******

#	Updates multivol files on other hosts
uqcspe $(HOSTS):	$(SOURCES) $(LIBSRC) $(HEADERS) $(MISC) $(MAKEFILE)
	echo "`date` % $@ $?" >> $@
	sendfile $(USER)@$@ $@ $?

#	Update all hosts
hosts:	$(HOSTS)

#	Retrieves sent files
getfile:
	getfile -DY

# Sorry, $* doesn't work in our version of make.
shar:	$(TARGET)1.shar $(TARGET)2.shar $(TARGET)3.shar $(TARGET)4.shar

$(TARGET)1.shar: $(MISC)
	$(SHAR) $(MISC) > $@ 

$(TARGET)2.shar: $(HEADERS) $(MAKEFILE) 
	$(SHAR) $(HEADERS) $(MAKEFILE) > $@ 

$(TARGET)3.shar: $(SRC1)
	$(SHAR) $(SRC1) > $@

$(TARGET)4.shar: $(SRC2)
	$(SHAR) $(SRC2) > $@

#	List files updated since last listing
list:	$(SOURCES) $(LIBSRC) $(HEADERS) $(MISC) $(MAKEFILE)
	echo "`date` % $@ $?" >> $@
	list $? &

ci:	$(SHSRC) $(SOURCES) $(HEADERS) $(MISC) $(MAKEFILE)
	ci -l -q $? </dev/null
	echo "`date` % $@ $?" >> $@

run:	$(TARGET)
	./$(TARGET)

vi:
	vi $(SOURCES) $(LIBSRC) $(HEADERS)

tags:	$(SOURCES) $(LIBSRC) $(HEADERS)
	ctags $(SOURCES) $(LIBSRC) $(HEADERS)
	
tidy:
	rm -f $(OUTDTA) LOG core a.out junk* $(TSTFIL) $(TSTFIL).[0-9] $(TSTSH) 

clean:		tidy
	rm -f $(OBJECTS) $(LIBOBJ) $(LIBRARY) testdata testdata.o

clobber:	clean
	rm -f $(TARGET)

lint:
	lint $(LINT) -hbx -lc $(SOURCES) $(LIBSRC) 2>&1 | tee LOG

install:	$(TARGET)
	mv $(TARGET) $(INSTALL)
	chmod 711 $(INSTALL)/$(TARGET)

err:
	error -v LOG

depend:
	sed -n -e '1,/^### DO NOT DELETE THIS LINE./p' < $(MAKEFILE) > $(MAKEFILE).new
	-for i in $(YACCFILES) $(LEXFILES) $(SOURCES) $(LIBSRC) $(FFILES) ; do\
		base=`expr $$i ':' '\(.*\).[cylf]$$'`;\
		suffix=`expr $$i ':' '.*\.\([cylf]\)$$'`;\
		if /bin/test $$suffix = l ; then\
			lex $$i;\
			mv lex.yy.c $$base.c;\
			suffix=c;\
			echo "$$base.c:	$$base.l" >> $(MAKEFILE).new;\
		elif /bin/test $$suffix = y ; then\
			yacc $(YFLAGS) $$i;\
			mv y.tab.c $$base.c;\
			suffix=c;\
			echo "$$base.c:	$$base.y" >> $(MAKEFILE).new;\
			echo "y.tab.h:	$$base.y" >> $(MAKEFILE).new;\
		fi;\
		$(CC) $(CLOCALFLAGS) -I$(INCLUDE) -E $$base.$$suffix |\
		grep '^# [0-9][0-9]* ".*"$$' > /tmp/grep$$$$;\
		sed -e 's/.*"\(.*\)"$$/\1/' -e 's/^.\///' < /tmp/grep$$$$ |\
		sort -u |\
		awk\
			"BEGIN { line=\"$$base.o:	\"}\
			{\
				if(length(line \$$0)>63)\
				{\
					print line,\"\\\\\";\
					line=\"		\"\$$0\
				}\
				else\
					line=line\" \"\$$0\
			}\
			END { print line}"\
		>> $(MAKEFILE).new;\
	done;\
	rm /tmp/grep$$$$
	mv $(MAKEFILE).new $(MAKEFILE)
### The following dependancies are/can be generated automatically
### by 'make depend'. Listen to this warning
###
### Do NOT put any of your own dependancies below this line,
### they will be removed
### DO NOT DELETE THIS LINE. USED FOR MAKE DEPEND
multivol.o:	 /usr/include/assert.h /usr/include/ctype.h \
		/usr/include/signal.h /usr/include/stdio.h \
		/usr/include/sys/param.h /usr/include/sys/types.h multivol.c \
		multivol.h options.h trace.h
rd.o:	 /usr/include/assert.h /usr/include/stdio.h multivol.h rd.c \
		trace.h
wr.o:	 /usr/include/assert.h /usr/include/ctype.h \
		/usr/include/errno.h /usr/include/stdio.h \
		/usr/include/sys/types.h multivol.h trace.h wr.c
vhdr.o:	 /usr/include/assert.h /usr/include/stdio.h \
		/usr/include/sys/types.h multivol.h trace.h vhdr.c
mount.o:	 /usr/include/assert.h /usr/include/ctype.h \
		/usr/include/stdio.h mount.c multivol.h trace.h
ask.o:	 /usr/include/stdio.h ask.c
error.o:	 /usr/include/stdio.h error.c
trace.o:	 trace.c
bcopy.o:	 bcopy.c
