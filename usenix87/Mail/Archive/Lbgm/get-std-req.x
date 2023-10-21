#! SHELL
# this program will take a file which has 'keywords', and extract the
# files corrosponding to those keywords
#
PATH=:.:/bin:/usr/bin:/usr/ucb:
export PATH
tmp=PROCESS/tmp.$$
tmp1=PROCESS/tmp1.$$
list=PROCESS/list.$$

trap "echo 'cleaning up'; /bin/rm -f $list $tmp $tmp1 $1.Work; exit 0" 0 1 2 3 9 15

if [ $# -ne 1 ]
then
    echo "Usage: `basename $0` file" 1>&2
    exit
fi

if [ $1 != "Mail" ]
then
    if [ -f $1.Work ]
    then
	echo "Work file exists for $1" 1>&2
	exit
    fi
    cp $1 $1.Work

    # construct the address to send to
    #
    ed - $1.Work <<'lasted'
/^END$/
+1,w address
1,/^BEGIN$/d
/^END$/,$d
w
q
lasted

    # lookup the line in the posted table of contents.
    while read line
    do
	grep -i "$line" TOCFROMSOURCEARCHIVE >> ${tmp}
    done < $1.Work

    # extract only those parts that fall into mod.sources
    sort -u ${tmp} > ${tmp1}

    # extract those lines which we really want
    ${EDITOR-${VISUAL-vi}} ${tmp1}
    mv ${tmp1} ${list}

    # now we can extract the files and stuff them into
    # 'destdir'
    destdir=PROCESS/outgoing
    ext=1

    # clear files for re-use

    /bin/rm -f ${tmp} ${tmp1}

    while read line
    do
	set `echo $line | awk -F: '{print $1 $3 $4 $5 $6}'`
	file=$1
	shift
	name=$1
	shift
	while [ $# -gt 0 ]
	do
	    name="${name}_${1}"
	    shift
	done
	xname=`echo $name |
	    tr '[\"\.\<\>\,\|\\\[\]{}]' '_' |
	    tr '[\`\!\@\#\$\%\^\&\*\(\)]' '_' |
	    tr "[\-\+\=\~\'\;\:\/\?\.]" "_" |
	    sed -e 's/^_//' -e "s/__/_/g" -e 's/_$//'`
	yname=`echo $xname | sed -e 's/__/_/g'`
	set `echo $file | awk -F_ '{printf("%s_%s %s_%s\n", $2, $1, $2, $3) }'`
	dir=$1
	prog=$2
	echo "${dir} ${prog} ${destdir}/${yname}.${ext}" >> ${tmp}
	ext=`expr ${ext} + 1`
    done < ${list}

    sort ${tmp} > ${tmp1}
    /bin/rm -f ${tmp}

    curmonth="FOO"
    curline="X"
    while true
    do
	if read line
	then
	    set ${line}
	    if [ $1 = ${curmonth} ]
	    then
		curline=`echo -n "${curline}$2 $3 "`
	    else
		if [ `expr length "${curline}"` -gt 1 ]
		then
		    echo ${curline} >> ${tmp}
		fi
		curmonth=$1
		curline="./get ${curmonth} $2 $3 "
	    fi
	else
	    echo ${curline} >> ${tmp}
	    break
	fi
    done < ${tmp1}

    sort -u ${tmp} > ${tmp1}
    #
    # do the extraction from the archive
    #
    while read line
    do
	(cd ARCHDIR ; ${line})
    done < ${tmp1}
fi
#
# mail the outgoing file
# validate the address
#
if [ ! -r address ]
then
    echo "Missing 'address'" 1>&2
    exit 1
fi

#
# show the mailing list
#
address=`cat address`
echo ""
echo "Mailing list -------------------------"
/bin/ls outgoing
echo "End Mailing list ---------------------"
echo "Mailing the above to '${address}'"
echo -n "is address correct? (y/n) [y] "
read ans
case "$ans" in
    n*|N*)
	${EDITOR-${VISUAL-vi}} ${address}
	;;
*)
	;;
esac

echo -n "Mail files now? (y/n) [y] "
read ans
case $ans in
    N*|n*)
	;;
    *)
	#
	# mail each piece of outgoing source
	# deleting it as we go.
	#
	for f in outgoing/*
	do
	    file=`basename $f`
	    echo "Mailing ${file}"
	    Mail -v -s "${file}" ${address} < ${f}
	    /bin/rm -f $f
	done

	# when all done, remove the address
	/bin/rm -f address
	;;
esac
