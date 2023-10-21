#! SHELL
# valid-log
#	Validate the contets of SAVELOG against the contents of
#	the directory.
#
#	SHOULD be un-necessary to run this program any more
#
if [ $# -ne 1 ]
then
	echo "Usage: $0 directory"
	exit 1
fi

cd ARCHDIR

ARCHIVE=$1
LOGFILE=SAVELOG
AWK=/tmp/awk$$
AWK1=/tmp/awk1$$
LS=/tmp/ls$$
MISSING=Missing

trap "/bin/rm $AWK $AWK1 $LS; exit 1" 1 2 3 9

cd $ARCHIVE

awk	'/^Jan_|^Feb_|^Mar_|^Apr_|^May_|^Jun_/
	 /^Jul_|^Aug_|^Sep_|^Oct_|^Nov_|^Dec_/' $LOGFILE > $AWK
sort $AWK > $AWK1 ; rm $AWK

/bin/ls *_* | grep -v '#' | sed -e 's/.Z//' > $LS
sed -e 's/://' $AWK1 | diff - $LS > $MISSING
if [ -s $MISSING ]
then
	echo "The following files are in the log, and do not have any source"
	grep -v '>' $MISSING | sed -e 's/>//' | more
	echo "The following files Are not in the log, and are in the archive"
	grep -v '<' $MISSING | sed -e 's/<//' | more
else
	echo "No missing files"
	/bin/rm -f $MISSING
fi
/bin/rm -f $AWK $AWK1 $LS

