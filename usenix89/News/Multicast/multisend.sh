: '@(#)multisend.sh	2.1	3/13/87'
#	Copyright (C) Shane McCarron, 1987.
#
#	This script is intended to be called only by multibatch.
#	Any parameters you give multibatch are passed onto this
#	script with the exception of the MULTICAST generated batch
#	file.
#
#	Note that the interface to multisend is pretty much like
#	sendbatch.  However, it does not support the ihave flags.
#	Also the flag -S is used to indicate that the rest of the
#	parameters are system names.  It is supplied by multibatch.


cflags=
LIM=50000
ECHO=
COMP=
C7=
RNEWS=rnews
date
echo $*

while [ $1 != '-S' ]
do
	case $1 in
	-[bBC]*)	cflags="$cflags $rmt"; continue;;
	-s*)	LIM=`expr "$rmt" : '-s\(.*\)'` ;;
	-c7) 	COMP='| LIBDIR/compress $cflags'
		C7='| LIBDIR/encode'
		ECHO='echo "#! c7unbatch"' ;;
	-c)	COMP='| LIBDIR/compress $cflags'
		ECHO='echo "#! cunbatch"' ;;
	-o*)	ECHO=`expr "$rmt" : '-o\(.*\)'`
		RNEWS='cunbatch' ;;
	esac
	shift
done

shift
FILE=$1
shift
SYSTEMS=$*

if test -n "$COMP"
then
	LIM=`expr $LIM \* 2`
fi

: make sure $? is zero
while test $? -eq 0 -a \( -s $FILE -o -s $FILE.work \)
do
	file=`date +10/16/86M%S`
	(eval $ECHO; eval LIBDIR/batch $FILE $LIM $COMP $C7) > /tmp/msend$$
	LIBDIR/uucast /tmp/msend$$ $RNEWS $SYSTEMS
	rm /tmp/msend$$
done
