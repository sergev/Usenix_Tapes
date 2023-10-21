# Makefile for shred
#
# To use this change MAKE_ROOT to reflect the correct path.
# You can also do this at the shell level by setting the enviroment:
# 	% setenv MAKE_ROOT the_path_to_follow
# The latter is advisable since if ever the directory is moved, only
# the environment need change for all Makefiles
#

DEST		= ${MAKE_ROOT}/bin
MAN		= ${MAKE_ROOT}/man/man1
LD		= ${CC}
CFLAGS		= -fsingle -O
LDFLAGS		= -fsingle
DEBUG		= -O
STRIP		= -s
INCLUDES	= 
LIBS		=
PROGRAM		= shred
EXECUTABLES	= ${PROGRAM}
OBJS		= ${PROGRAM}.o
SRCS		= $(PROGRAM).l

all:	${PROGRAM}

${PROGRAM}:	${OBJS} ${LIBS}
		${LD} ${LDFLAGS} ${OBJS} ${LIBS} -o ${PROGRAM}

clean:;		@rm -f core junk foo *.out *.o *.lint
		@rm -f ${EXECUTABLES}

install: ${EXECUTABLES} ${SCRIPTS} manual
	 cp ${EXECUTABLES} ${SCRIPTS} ${DEST}

man:	manual

manual:
	-cp ${PROGRAM}.[12345678]* ${MAN}

print:	.print

.print:	${SRCS} ${MENU} ${SCRIPTS} 
	itroff -t -ms -man ${PROGRAM}.[1234]? | catimp -h
	vgrind -t $? | catimp -h
	touch .print

lint:	${SRCS} 
	lint ${INCLUDES} $?
