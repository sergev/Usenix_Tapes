#! SHELL
# OBSOLETE
# modlist [ARCHIVE [LOGFILE]]
#	generate a listing of the moderated sources which
#	have been archived
# use this to print out a simple table of contents for the
# lbgm archive

trap "/bin/rm -f $modlist; exit 0" 0 1 2 3 4 5 6 7 8 9

if [ `whoami` != "news" ]
then
	echo "Only news may run this program"
	exit 1
fi

modlist=/tmp/modlist.execute
cat > $modlist << 'eof-modlist'
#! SHELL
LOGFILE=SAVELOG
ARCHIVE_BASE=ARCHDIR

SHOW_HEADER=0

if [ $# -gt 2 ]
then
	SHOW_HEADER=1
fi

while [ $# -gt 0 ]
do
	YEAR=$1
	DIR=$2
	cd ${ARCHIVE_BASE}/${DIR}
	shift
	shift

	if [ $SHOW_HEADER -eq 1 ]
	then
		echo "========== $DIR =========="
	fi

	awk 'BEGIN { doprint=0; file="" }
		/^Jan_|^Feb_|^Mar_|^Apr_|^May_|^Jun_/ {file = $0}
		/^Jul_|^Aug_|^Sep_|^Oct_|^Nov_|^Dec_/ {file = $0}
		/^	Subject/	{ subj = $0 }
		/^	Newsgroups/	{ if ( $2 ~ /mod.sources/ )
						doprint=1
					}
		/^	$/ { if ( doprint == 1 )
				printf "%s_%s   %s\n", "'"$YEAR"'",file,subj
				doprint=0
			   }
		' $LOGFILE

	if [ $SHOW_HEADER -eq 1 ]
	then
		echo "========== $DIR =========="
	fi

done
eof-modlist

chmod 755 $modlist

DIRLIST=""

for year in 4 5 6
do
	for month in Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
	do
		DIR=${month}_8${year}
		if [ -d $DIR ] ; then
			DIRLIST="$DIRLIST 8${year} $DIR"
		fi
	done
done

set `date`
DIRLIST="$DIRLIST $6 Current"

$modlist $DIRLIST
