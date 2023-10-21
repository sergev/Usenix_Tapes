
#! /bin/sh
#
# compress/uncompressdir
#
#	Tom Rushworth
#	Stuart Lynne
#					Mar/87
#
#
#	Modified from compress 4.0 release
#
OPTIONS=
FILES=
if test `basename $0` = "compressdir"; then
EFLAG=-v
COMPRESS=compress
else
EFLAG=
COMPRESS=uncompress
fi
for ARG
do
	case "$ARG" in
	-*)	OPTIONS="$OPTIONS $ARG";;
	*)	FILES="$FILES $ARG";;
	esac
done
if test -z "$FILES"; then
	FILES="."
fi
set $FILES
find $@ -type f -links 1 -print |
	egrep $EFLAG '\.Z$' |
	while read FILE
	do
		if test -r $FILE -a -s $FILE
		then
			$COMPRESS $OPTIONS $FILE
		fi
	done

-- 
Stuart Lynne	ihnp4!alberta!ubc-vision!van-bc!sl     Vancouver,BC,604-937-7532
