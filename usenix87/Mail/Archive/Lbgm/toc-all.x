#! SHELL
# toc-all
#	used to print out a complete listing of the table of contents for
#	all past months.
#

trap "exit 0" 0 1 2 3 4 5 6 7 8 9

cd ARCHDIR

TOC=/usr/lib/news/archive/toc

case `whoami` in
news|MYNAME)
	;;
*)
	echo "Only news may run this program"
	exit 1
	;;
esac

DIRLIST=""

for year in 83 84 85 86
do
	for month in Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
	do
		DIR=${month}_${year}
		if [ -d $DIR ] ; then
			DIRLIST="$DIRLIST $DIR ${year}_ "
		fi
	done
done

#set `date`
$TOC $DIRLIST
