#! SHELL
# toc
#	use this to print out a simple table of contents for the
#	lbgm archive
#	can be used to print out entire list of contents when given
#		'toc dir year [ dir year ... ]'

cd ARCHDIR

if [ $# -lt 1 ]
then
	echo "Please indicate Month" 2>&1
	exit 1
fi

HEAD_TRAIL=0

while [ $# -gt 0 ]
do
	YEAR=""
	ARCHIVE=$1
	shift
	if [ $# -gt 0 ]
	then
		case "$1" in
		Current)
			HEAD_TRAIL=1
			YEAR=""
			;;
		Jan_*|Feb_*|Mar_*|Apr_*|May_*|Jun_*)
			HEAD_TRAIL=1
			YEAR=""
			;;
		Jul_*|Aug_*|Sep_*|Oct_*|Nov_*|Dec_*)
			HEAD_TRAIL=1
			YEAR=""
			;;
		*)
			YEAR=$1
			HEAD_TRAIL=0
			shift
		esac
	fi

	LOGFILE=SAVELOG
	cd $ARCHIVE

	if [ $HEAD_TRAIL -eq 1 ]
	then
		echo "========== $ARCHIVE =========="
	fi

	awk 'BEGIN { file="" }
		/^Jan_|^Feb_|^Mar_|^Apr_|^May_|^Jun_/ {file = $0}
		/^Jul_|^Aug_|^Sep_|^Oct_|^Nov_|^Dec_/ {file = $0}
		/^	Subject/	{ subj = $0 }
		/^	Newsgroups:/	{
					  ex=0
					  star=0
					  if ($2 ~ /net.sources/) {
					  }
					  else {
					    if ($2 ~ /mod.sources/ ) {
					      star=1
					    }
					    else {
					      ex=1
					    }
					  }
					}
		/^	$/ {
				if (star == 1) {
				  printf "%s%s *%s\n","'"$YEAR"'",file,subj
				}
				else {
				  if (ex == 1) {
				    printf "%s%s x%s\n","'"$YEAR"'",file,subj
				  }
				  else {
				    printf "%s%s  %s\n","'"$YEAR"'",file,subj
				  }
				}
			   }
	' $LOGFILE

	if [ $HEAD_TRAIL -eq 1 ]
	then
		echo "========== $ARCHIVE =========="
	fi

	cd ..
done
