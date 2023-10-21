#! /bin/sh
# mved.sh
# Move-and-edit filenames.
#
#	Usage: mved [-n] from-pattern to-pattern
#
#	This command allows you to change file names much as is possible
#	with some versions of PIP (remember *.txt=*.bak?).
#	The '=' character in from-pattern is treated as a special wildcard,
#	matching in the same way as the shell '*' wildcard character, except
#	that the text matching the '=' in the first pattern is inserted in
#	place of any = wildcards in the second.
#	Note that from-pattern need not have a wildcard if to-pattern does,
#	a default 
#
#	Use the '-n' option to do nothing, showing what would be done.
#
#	Restrictions:
#	Only the first '=' sign in from-pattern is used.  Multiple =
#	wildcards in from-pattern match up with the first from-pattern
#	=, ie: there is no matching for multiple = signs.  (I'm sure
#	someone could make it work if they wanted to... ?)
#
#   eg: mved lib=.a =.a		moves   libhello.a to hello.a
#	mved =.o =.o.old		moves	fred.o to fred.o.old
#	mved '=.*' =		moves	fred.junk to fred
#	mved =.sh =		moves	mved.sh to mved
#	mved *.sh =.
#
#			Brian Coogan 06 Jan 87
#			Hewlett-Packard Australian Software Operation
# $Header$ ASO

shopt=x
case "$1" in
-n)	shopt=vn; shift ;;
esac

# Check for appropriate wildcards.
# Source must have an = or a * wildcard or already exist.
case "$1" in
*=*)	;;
*)	for n in $1
	do if [ ! -f "$n" ]
	then
		echo "$0: No files match from-pattern!\n" 1>&2
		set -- "$@" give usage message
	elif [ "$2" = '=' ]
	then
		echo Nothing doing.
		exit 0
	fi
	break
	done
	;;
esac
case "$2" in
*=*)	;;
*)	echo "$0: No '=' wildcards used in target!\n" 1>&2
	set -- "$@" give usage message
	;;
esac
# catch mved = =
case "$1$2" in
==)	echo Nothing doing.; exit 0;;
esac



if [ $# -ne 2 ]
then
	echo "Usage: $0 [-n] from-pattern to-pattern" 1>&2
	echo "\tEquals (=) signs in the to-pattern match like '*' and are"
	echo "\treplaced with the text that matched the = in from-pattern."
	echo "\tYou must quote any '*'s in from-pattern."
	exit 1
fi

globpatt=`echo $1 | sed 's/=/\*/'`

frompatt=`echo "$1" | sed \
			-e 's/\./\\\\./g' \
			-e 's/\*/.*/g' \
			-e 's/=/\\\\(\\.\\*\\\\)/' \
			-e '/\\\\(/ !s/.*/\\\\(&\\\\)/'
			`
topatt=`echo "$2" | sed -e 's/=/\\\\1/g'`


for n in $globpatt
do
	# Check the pattern got expanded.  (The file might also have vanished).
	if [ ! -f $n ]
	then
		echo "$0: No files matching $1 found." 1>&2
		exit 1
	fi
	echo $n
done |
sed -n "s;$frompatt;mv & $topatt;p" | sh -$shopt

echo done
