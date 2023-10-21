#! SHELL
: '/*********************************************************************
	savesource
		Save sources sent over net.sources.
		Called as a news forwarding program from inews

	Alan S. Watt
   
	Sccsid="@(#)savesource	1.1 8/15/82 (ITT)"

	usage:
		savesource [ savedir ]

	arguments:
		savedir		Directory area to save sources (default
				is /usr/lbgm).
   
	history:
		08/15/82	original version
   *********************************************************************/'

: 'Public source directory (Little Bird Gave Me)'
ARCHIVE=ARCHCUR
sourcedir=${1-"$ARCHIVE"}

: 'format of date command: "Sun Aug 15 15:56:06 EDT 1982"'
set `date`
mon=$2
day=$3
uniq=$$
tmp=svsrc.$$.tmp
logfile=SAVELOG

: 'Lock the save source file and dump standard input there'
cd $sourcedir
>$tmp
while savefile="${mon}_${day}.$uniq"
do
	if ln $tmp $savefile 2>&-
	then
		break
	fi
	uniq=`expr $uniq + 1`
done
cat >${savefile}
rm -f $tmp

: 'Dump the news header in a log file (up to a blank line)'
header="$savefile:
`sed -n '1,/^$/s/^/	/p' ${savefile}`"
echo "${header}" >>$logfile
