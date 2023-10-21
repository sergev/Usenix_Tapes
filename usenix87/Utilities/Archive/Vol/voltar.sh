: voltar.sh
#
# Run vol to create volume header files.
# Run tar with the volume header files.
#

BEEP=""

TAPECMD="tar cvbf 126 /dev/rct"
DISKCMD="tar cv"

TAR_CMD=$DISKCMD
OPTIONS="-d"
PROMPT="floppy diskette"

FILES=

if test $# -eq 0
then echo "Usage: voltar [ -dt ] pathnames..."; exit
fi

for i
do
	case $i in

		-d)	TAR_CMD=$DISKCMD
			OPTIONS="-d"
			PROMPT="floppy diskette"
			;;

		-t)	TAR_CMD=$TAPECMD
			OPTIONS="-t"
			PROMPT="magnetic tape"
			;;

		*)	FILES="$FILES $i"
			;;
	esac
done

rm -f Vol_*
vol $OPTIONS $FILES

for i in Vol_*
do
	echo $BEEP
	echo -n "$i: Insert $PROMPT, then type <return>: "
	read line
	$TAR_CMD `cat $i`
done
