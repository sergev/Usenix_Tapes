#! SHELL
# squish [ dir ]
#	used to compress the data in the 'past month' of sources.
#	run on the first of the month to compress past month, and
#	in conjunction with creating a new month.
#

cd ARCHDIR

LOGFILE=SAVELOG
if [ $# -ne 1 ]
then
	echo "Usage: $0 dir"
	exit 1
fi

if [ $1 = "Current" ]
then
    echo "`basename $0`: 'Current' invalid directory"
    exit
elif [ `/bin/ls -l ARCHCUR | sed -e 's/\(.*\) -> //'` = $1 ]
then
    echo "`basename $0`: '$1' invalid directory (Really Current)"
    exit
fi

if [ ! -d $1 ]
then
	echo $1 is not a directory
	exit 1
fi

cd $1
compress *_*
echo Shrink $LOGFILE
LOGFILE=SAVELOG
awk 'BEGIN { file="" }
	/^Jan_|^Feb_|^Mar_|^Apr_|^May_|^Jun_/ {file = $0}
	/^Jul_|^Aug_|^Sep_|^Oct_|^Nov_|^Dec_/ {file = $0}
	/^	Subject:/	{sub=$0}
	/^	Message-ID:/	{mid=$0}
	/^	Path:/		{path=$0}
	/^	Newsgroups:/	{ng=$0}
	/^	From:/		{from=$0}
	{ if ( $0 ~ /^[	 ]*$/ )
		printf "%s\n%s\n%s\n%s\n%s\n%s\n\t\n", \
			file, mid, sub, from, path, ng}' $LOGFILE > $LOGFILE.$$
/bin/mv $LOGFILE.$$ $LOGFILE
