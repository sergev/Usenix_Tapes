#
#  Makefile for the Elm system utilities
#
#         (C) Copyright 1986, Dave Taylor
#
#  Last modification: Sept 15th, 1986

SHELL=/bin/sh

DEFINE= 
LIB2  = 

CFLAGS= -O -I../hdrs
CC=	/bin/cc
RM= 	/bin/rm -f
ECHO=  /bin/echo

OBJS=	../bin/newalias ../bin/from ../bin/newmail ../bin/answer       \
	../bin/printmail ../bin/fastmail ../bin/readmsg                \
	../bin/checkalias ../bin/arepdaemon ../bin/autoreply ../bin/wnewmail \
	../bin/messages ../bin/trim-headers ../bin/listalias 

all: ${OBJS}

../bin/newalias:  ../hdrs/defs.h newalias.c ../src/validname.o \
	../src/opt_utils.o
	${CC} ${CFLAGS} ${DEFINE} newalias.c ../src/validname.o \
	../src/opt_utils.o -o ../bin/newalias 

../bin/from: from.c ../src/opt_utils.o ../src/string2.o
	${CC} ${CFLAGS} ${DEFINE} from.c ../src/opt_utils.o \
	../src/string2.o -o ../bin/from

../bin/newmail: ../src/opt_utils.c newmail.c ../src/string2.o
	${CC} ${CFLAGS} ${DEFINE} newmail.c \
	../src/string2.o -o ../bin/newmail

../bin/wnewmail: ../src/opt_utils.c wnewmail.c ../src/string2.o
	${CC} ${CFLAGS} ${DEFINE} ../src/opt_utils.o \
	../src/string2.o wnewmail.c -o ../bin/wnewmail

../bin/listalias: listalias.c 
	${CC} ${CFLAGS} ${DEFINE} listalias.c -o ../bin/listalias

../bin/answer: answer.c ../src/opt_utils.o
	${CC} ${CFLAGS} ${DEFINE} answer.c ../src/opt_utils.o -o ../bin/answer

../bin/printmail: printmail.c ../src/opt_utils.o
	${CC} ${CFLAGS} ${DEFINE} printmail.c ../src/opt_utils.o \
	-o ../bin/printmail

../bin/fastmail: fastmail.c 
	${CC} ${CFLAGS} ${DEFINE} fastmail.c ../src/opt_utils.o \
	-o ../bin/fastmail

../bin/readmsg: readmsg.c ../src/getopt.o ../src/opt_utils.o ../src/string2.o
	${CC} ${CFLAGS} ${DEFINE} readmsg.c ../src/getopt.o ../src/string2.o \
	../src/opt_utils.o -o ../bin/readmsg

../bin/arepdaemon: arepdaemon.c
	${CC} ${CFLAGS} ${DEFINE} arepdaemon.c -o ../bin/arepdaemon

../bin/autoreply: autoreply.c ../src/opt_utils.o
	${CC} ${CFLAGS} ${DEFINE} autoreply.c ../src/opt_utils.o \
	-o ../bin/autoreply

../bin/checkalias: 
	@echo ': Use /bin/sh' > ../bin/checkalias
	@echo 'if [ "$$*" = "" ]; then' >> ../bin/checkalias
	@echo '  echo Usage: checkalias alias \[or aliases\]' >>  \
	      ../bin/checkalias
	@echo '  exit 1' >> ../bin/checkalias
	@echo 'fi' >> ../bin/checkalias
	@echo ' '  >> ../bin/checkalias
	@echo 'exec elm -c $$*' >> ../bin/checkalias
	@chmod +x ../bin/checkalias

../bin/messages: 
	@echo ': Use /bin/sh' 				 > ../bin/messages
	@echo 'if [ "$$2" != "" ]; then' 		>> ../bin/messages
	@echo '  echo Usage: messages \{folder-name\}'  >> ../bin/messages
	@echo '  exit 1' 				>> ../bin/messages
	@echo 'fi' 					>> ../bin/messages
	@echo ' '  					>> ../bin/messages
	@echo 'if [ "$$1" = "" ]; then' 		>> ../bin/messages
	@echo '  fname=$$MAIL' 				>> ../bin/messages
	@echo '  optional="in your mailbox"'  		>> ../bin/messages
	@echo 'else'		 			>> ../bin/messages
	@echo '  fname=$$1'				>> ../bin/messages
	@echo '  optional="in folder $$1"' 		>> ../bin/messages
	@echo 'fi'	       				>> ../bin/messages
	@echo ' '					>> ../bin/messages
	@echo 'mcount=`egrep "^From " $$fname | wc -l`' >> ../bin/messages
	@echo ' '					>> ../bin/messages
	@echo 'if [ $$mcount = 1 ];then'		>> ../bin/messages
	@echo '  echo There is $$mcount message $$optional' >> ../bin/messages
	@echo 'else'					>> ../bin/messages
	@echo '  echo There are $$mcount messages $$optional' >> ../bin/messages
	@echo 'fi'					>> ../bin/messages
	@echo ' '					>> ../bin/messages
	@echo 'exit 0'					>> ../bin/messages
	@chmod +x ../bin/messages

../bin/trim-headers:
	@cp trim-headers ../bin/trim-headers
	@chmod +x ../bin/trim-headers

../src/validname.o: ../src/validname.c ../hdrs/defs.h
	@(cd ../src; ${CC} -c ${CFLAGS} ${DEFINE} validname.c; cd ../utils)

../src/opt_utils.o: ../src/opt_utils.c ../hdrs/defs.h
	@(cd ../src; ${CC} -c ${CFLAGS} ${DEFINE} opt_utils.c; cd ../utils)

../src/getopt.o: ../src/getopt.c ../hdrs/defs.h
	@(cd ../src; ${CC} -c ${CFLAGS} ${DEFINE} getopt.c; cd ../utils)

../src/string2.o: ../src/string2.c ../hdrs/defs.h
	@(cd ../src; ${CC} -c ${CFLAGS} ${DEFINE} string2.c; cd ../utils)

clean:
	${RM} *.o ${OBJS} ../bin/utils

lint:
	lint -p -I../hdrs *.c > LINT.OUT
