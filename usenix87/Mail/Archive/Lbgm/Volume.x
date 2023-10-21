#! MYHOME/bin/jsh -j
# Volume: extract the volume number from the file
#
NEWSARCH=/usr/lib/news/archive
ALLTOC=PROCESS/TOCFROMSOURCEARCHIVE
MODTOC=/tmp/MOD-TOC
VOL=/tmp/vvol$$
TMP=/tmp/vtmp$$

# get to the archive directory
cd ARCHDIR

trap "/bin/rm -f ${VOL} ${TMP} ${MODTOC}; exit" 0 1 2 3 9 15

if [ ! -f ${ALLTOC} ]
then
	echo "Please wait Running 'toc-all'"
	${NEWSARCH}/toc-all > ${ALLTOC}
fi

grep '*' ${ALLTOC} > ${MODTOC}

while read LINE
do
	# get the directory/filename to work with
	XFILE=`echo $LINE |\
		awk -F_ '{printf "%s_%s/%s_%s", $2, $1, $2, $3}'`
	# get out the filename from the entire list
	FILE=`echo $XFILE | sed -e 's/\(.*\):\(.*\)$/\1/'`
	# check for compressed file, or regular file
	if [ -f "$FILE".Z ]
	then
		zcat $FILE > $TMP &
	else
		head -45 $FILE > $TMP
	fi
	# extract the title
	TITLE=`echo $LINE | sed -e 's/^\(.*\)Subject:/Subject:/'`
	echo "$FILE: $TITLE"
	# if compressed file wait for compression to finish
	wait
	# find volume
	grep "Volume" $TMP > $VOL 2>&1
	# if grep returns non zero status, No volume, else
	# we have a volume
	if [ $? != 0 ]
	then
		echo "$FILE: No volume"
	else
		echo -n "$FILE: "
		cat $VOL
	fi
	/bin/rm $VOL $TMP
done < ${MODTOC}
