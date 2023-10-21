#! SHELL
#
# mail the outgoing file
# validate the address
#
erroruser=MYNAME

true="[ 1 = 1 ]"
false="[ 1 = 0 ]"

# if in background, then verbose is false, mailinglist is false.
# mailinglist == show the mailing list
verbose=false
veryverbose=false
mailinglist=false
#if debugging, verbose is true
debug=false
if ${debug}
then
    verbose=true
    veryverbose=true
    mailinglist=true
fi
useucb=true
ucbmail=/usr/ucb/Mail
mhsend=/usr/new/mh/send

# if you want to restrict the size of a mail message, use
# this, setting checksize to true.
checksize=true
ls=/bin/ls
field=4
maxsize=64500

request=$1
addressfile=$2
processdir=$3
prog=$4

destdir=${processdir}/out-${addressfile}

if [ ! -r ${addressfile} ]
then
    echo "Missing '${addressfile}'" | Mail -s "Bad request" ${erroruser}
    exit 1
fi

if [ ! -d ${destdir} ]
then
    echo "Missing directory '${destdir}'" | Mail -s "Bad Request" ${erroruser}
    exit 1
fi

#
# show the mailing list
#
if ${mailinglist}
then
    echo "Mailing list -----------------------------"
    /bin/ls ${destdir}
    echo "End Mailing list -------------------------"
fi
if ${checksize}
then
    again=1
    while [ ${again} = 1 ]
    do
	for i in ${destdir}/*
	do
	    again=0
	    size=`/bin/ls -l $i | awk '{print $'"${field}"'}'`
	    if [ ${size} -gt ${maxsize} ]
	    then
		# split the file in half, and then look at all the
		# files again.  This means that any file which is
		# more than 3x ${maxsize} will wind up with names
		# like x.1.aa.aa.aa x.1.aa.aa.ab x.1.aa.aa x.1.aa.ab
		again=1
		filename=$i
		set `wc -l ${filename}`
		half=`expr $1 '/' 2`
		half=`expr ${half} + 100`
		split -${half} ${filename} ${filename}.
		/bin/rm ${filename}
		break
	    fi
	done
    done
fi
address=`cat ${addressfile}`
if [ -z ${address} ]
then
    echo "Empty address" | Mail -s "Bad Request" ${erroruser}
    exit 1
fi

if false
then
    echo "Mailing the above to '${address}'"
    echo -n "is address correct? (y/n) [y] "
    read ans
    case "$ans" in
	n*|N*)
	    vi ${addressfile}
	    address=`cat ${addressfile}`
	    ;;
	*)
	    ;;
    esac
fi
#
# mail each piece of outgoing source
# deleting it as we go.
#
for outfile in ${destdir}/*
do
    file=`basename ${outfile}`
    if ${verbose}
    then
	echo "Mailing ${file}"
    fi
    if ${useucb}
    then
	# keep the '-v' here, it reduces the overall load on
	# the system.
	if ${veryverbose}
	then
	    ${ucbmail} -v -s "${file}" ${address} < ${outfile}
	else
	    (${ucbmail} -v -s "${file}" ${address} < ${outfile})>/dev/null 2>&1
	fi
    else
	mhout=/tmp/OUT_FILE.$$
	echo "To: ${address}" > ${mhout}
	echo "Subject: ${file}" >> ${mhout}
	echo "--------" >> ${mhout}
	cat ${outfile} >> ${mhout}
	if ${veryverbose}
	then
	    ${mhsend} ${mhout} -verbose -watch
	else
	    (${mhsend} ${mhout}) > /dev/null 2>&1
	fi
    fi
    /bin/rm -f ${outfile}
done

# when all done, remove the address
/bin/rm -f ${addressfile}
/bin/rm -rf ${destdir}

ext=1
while [ -f done/${request}.${ext} ]
do
    ext=`expr ${ext} + 1`
done
mv ${request} done/REQ.${addressfile}.${ext}
