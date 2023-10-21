#! /bin/sh
#
# maketd - generates file dependencies for makefiles using cc -M
#
PATH=/usr/local/bin:/bin:/usr/bin:/usr/ucb
export PATH

progname=`basename $0`

# Name of the Makefile which will be edited to add the dependencies
if [ $# = 0 ] ; then
    cat << EOF
usage: $progname [-a] [-d] [-m<file>] [-o<directory>] [-D...] [-I...] [-U...]
	      [<file> ...] [-T <file> ...]
EOF
    exit
fi

DEPFILE=/tmp/mtd3$$.tmp
touch $DEPFILE
EDDFILE=/tmp/mtd4$$.tmp

trap 'rm -f $DEPFILE $EDDFILE ; exit ' 1 2 3 15

# Default values for -a, -d, -m and -o options
AOPTION="-e /\/usr\/include/d"
DOPTION=0
MAKEFILE=Makefile
OBJDIR=

# Collect in OPTIONS all options you want to pass on to the C preprocessor.
# 	  in SOURCES all files you want to create dependencies for
while [ -n "$1" ] ; do
    case $1 in

	-a)
	    AOPTION=
	;;

	-d)
	    DOPTION=1
	;;

	-m*)
	    MAKEFILE=`expr $1 : '-m\(.*\)'`
	;;

	-nonlocalo)
	    echo "$progname: -nonlocalo option obsolete"
	    exit 1
	;;

	-o*)
	    if [ "$1" = "-o" ] ; then
		echo    "$progname: -o option requires directory name"
		exit 1
	    fi
	    OBJDIR=`expr $1 : '-o\(.*\)'`
	    if [ ! -d $OBJDIR ] ; then
		echo    "$progname: -o option: \"$OBJDIR\" is not a directory"
		exit 1
	    fi
	    OBJDIR="$OBJDIR/"
	;;

	-[D,I,U]*)
	    OPTIONS="$OPTIONS $1"
	;;

	-T)
	    shift
	    TSOURCES="$*"
	    set ""
	;;

	-*)
	    echo        "$progname: option \"$1\" unknown; ignored"
	;;

	*)
	    SOURCES="$SOURCES $1"
	;;
    esac
    shift
done

#       Run everything through the preprocessor (using cc), sort this
#	output and remove duplictate lines.  If there is no '-a' option
#	remove all dependencies of the form '/usr/include/<file>'. Cc
#	will exit quietly if there are no source files.
/bin/cc -M $OPTIONS $SOURCES | sort | uniq | \
    sed $AOPTION \
	-e "s, \./, ,g" \
	-e "s,\.\./[a-zA-Z0-9]*/\.\.,\.\.,g" \
	-e "s,^,$OBJDIR," >> $DEPFILE

/bin/cc -M $OPTIONS $TSOURCES | sort | uniq | \
    sed $AOPTION \
	-e "s,\.o:,:," \
	-e "s, \./, ,g" \
	-e "s,\.\./[a-zA-Z0-9]*/\.\.,\.\.,g" \
	-e "s,^,$OBJDIR," >> $DEPFILE

# If DOPTION then cat file and exit, otherwise edit Makefile
if [ $DOPTION -eq 1 ] ; then
    cat $DEPFILE
    rm -f $DEPFILE $EDDFILE
    exit
fi

# Now start editing the Makefile
if [ ! -w $MAKEFILE ] ; then
    echo        "$progname: can't edit $MAKEFILE"
    rm -f $DEPFILE $EDDFILE
    exit 1
fi


# Make sure we have the "DO NOT DELETE" line in the Makefile
cat << EOF >> $MAKEFILE

# DO NOT DELETE THIS LINE - maketd DEPENDS ON IT
EOF

# Build the editor script to edit the Makefile later.
cat << EOF > $EDDFILE
/# DO NOT DELETE THIS LINE/,\$d
\$a
# DO NOT DELETE THIS LINE - maketd DEPENDS ON IT
# Dependencies generated at: `date`

EOF
cat $DEPFILE >> $EDDFILE
cat << EOF >> $EDDFILE

# DO NOT ADD ANYTHING HERE - WILL GO AWAY
.
w
q
EOF

# Editor scipt done, edit makefile now.
cp $MAKEFILE $MAKEFILE.bak
ed - $MAKEFILE < $EDDFILE

rm -f $DEPFILE $EDDFILE
