#! SHELL
# get-mod-req: Process the request, leaving the outgoing files in
#	the subdirectory 'outgoing', and the file 'address' with
#	where to send the files
#
PATH=:.:/bin:/usr/bin:/ucb:${PATH}
export PATH

prog=$0
case $# in
    3)
	processdir=$1
	request=$2
	address=$3
	;;
    *)
	echo "Usage: `basename ${prog}` dir file" 1>&2
	exit 1
	;;
esac

if [ ! -d ${processdir} ]
then
    echo "'${processdir}' is not a directory"
    exit 1
fi

true="[ 1 = 1 ]"
false="[ 0 = 1 ]"

#
# system dependant macros
#
archive=ARCHDIR		# where the sources archive lives
destdir=${processdir}/out-${address}	# change this for your system
MOD=${processdir}/TOCFROMMODSOURCES	# file which has the posted TOC from mod.sources
ALL=${processdir}/TOCFROMSOURCEARCHIVE	# total table of contents for entire archive
lookupwords=${processdir}/lookup-words # list of words looked-up
ignorewords=${processdir}/skip-words # list of words to ignore in keyword build
mailprog=${processdir}/do-mail	# back end which does mailing of outgoing srcs
grep=${processdir}/bm	# could be 'grep' as well

if [ ! -d ${destdir} ]
then
    if [ ${request} = "Mail" ]
    then
	echo "'${destdir}' missing for 'Mail' request" 1>&2
	exit 1
    else
	mkdir ${destdir}
	chmod 777 ${destdir}
    fi
elif [ ${request} != "Mail" ]
then
    echo "'${destdir}' exists!" 1>&2
    exit 1
fi

#
# boolean parameters
#
verbose=${true}		# talk to us
veryverbose=${false}	# LOTS
nooptimize=${true}	# don't optimize the archive extraction
modonly=${false}	# we want all references
nodebug=${true}		# we are 'debugging'
savelookup=${true}	# save the list of words we are looking up

#
# list of temp files
#
tmp=${processdir}/tmp.$$
tmp1=${processdir}/tmp1.$$
list=${processdir}/list.$$

#
# validate that important things are here
#
if [ ! -f ${mailprog} ]
then
    echo "${mailprog} is missing" 1>&2
    exit 1
fi
if [ ! -d ${archive} ]
then
    echo "'${archive}' is not a directory or is missing" 1>&2
    exit 1
fi
if [ ! -d ${destdir} ]
then
    echo "'${destdir}' is not a directory or is missing" 1>&2
    exit 1
fi
if [ ! -r ${MOD} ]
then
    echo "'${MOD} is not readable or is missing" 1>&2
    exit 1
fi
if [ ! -r ${ALL} ]
then
    echo "'${ALL}' is not a readable or is missing" 1>&2
    exit 1
fi

if [ ${request} = "Mail" ]
then
    exec ${mailprog} ${request} ${address} ${processdir} ${prog}
fi

if ${veryverbose}; then verbose=${true}; fi

if ${verbose}; then echo -n "Setup, "; fi

if [ ! -r ${request} ]
then
    echo "Can't find ${request}" 1>&2
    exit 1
fi

if [ -f ${request}.Work ]
then
    echo "Work file exists for ${request}" 1>&2
    exit 1
fi

if ${nodebug}
then
    trap "echo 'cleaning up'; /bin/rm -f $list $tmp $tmp1 ${request}.Work ; exit 1" 1 2 3 9 15
fi

if ${veryverbose}; then echo -n "Create address, "; fi

cp ${request} ${request}.Work

# construct the address to send to
#
ed - ${request}.Work <<'lasted' 2>/dev/null
/^END$/
+1,w address.tmp
1,/^BEGIN$/d
/^END$/,$d
w
q
lasted

mv address.tmp ${address}
/bin/rm -f ${tmp}

if ${verbose}; then echo echo -n "Lookup from posted moderated TOC, "; fi

# lookup the line in the posted table of contents.
while read line
do
    if ${veryverbose}; then echo -n "${line}, "; fi
    grep -i "^${line}" ${MOD} >> ${tmp}
done < ${request}.Work

if ${verbose}; then echo "done"; echo -n "Create keyword list, "; fi

# split line, remove blank lines, and then continue
sed -e 's/-/ /g' < ${tmp} | tr ' \011' '\012' |\
	sed -e '/^$/d' | egrep -v '(part|program|\(|\))' > ${tmp1}

#
# lookup all the parts of the lines
# in a case independent fashion.
#
/bin/rm -f ${tmp}
while read line
do
    xline=`echo ${line} | tr -d '[0-9]\.'`
    grep -s -i ${xline} ${ignorewords}
    if [ $? != 0 ]
    then
	if ${veryverbose}; then echo -n "${xline}, "; fi
	echo ${xline} >> ${tmp}
    fi
done < ${tmp1}

if ${savelookup}
then
    echo "-------" >> ${lookupwords}
    cat ${tmp} >> ${lookupwords}
fi

if ${verbose}; then echo "done"; echo -n "Lookup files from complete TOC, "; fi
fgrep -i -f ${tmp} ${ALL} > ${tmp1}

if ${verbose}; then echo "done"; fi

#
# extract only those parts that fall into mod.sources
# if the modonly flag is set
#
if ${modonly}
then
    ${grep} '*' ${tmp1} | sort -u > ${list}
else
    sort -u ${tmp1} > ${list}
fi

# extract those lines which we really want
${EDITOR-${VISUAL-vi}} ${list} ${request}

# clear files for re-use

/bin/rm -f ${tmp} ${tmp1}

if ${verbose}; then
    if ${nooptimize}; then
	echo -n "Create extract list, "
    else
	echo -n "Create raw extract list, "
    fi
fi


ext=1
#
# now we can extract the files and stuff them into 'destdir'.
# Foreach line from the table of contents, process it into
# something which can be 'extracted' from the archive.
#
while read line
do
    if ${veryverbose}; then echo -n "${line}, "; fi
    leng=`expr length "${line}"`
    if [ ${leng} -lt 12 ]
    then
	echo "Line '${line}' too short, skipped" 1>&2
    else
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
	    sed -e 's/^_//' -e 's/__/_/g' -e 's/_$//'`
	yname=`echo $xname | sed -e 's/__/_/g'`
	set `echo $file | awk -F_ '{printf("%s_%s %s_%s\n", $2, $1, $2, $3) }'`
	dir=$1
	prog=$2
	if ${nooptimize}
	then
	    echo -n "./get " >> ${tmp}
	fi
	echo "${dir} ${prog} ${destdir}/${yname}.${ext}" >> ${tmp}
	ext=`expr ${ext} + 1`
    fi
done < ${list}

if ${verbose}; then echo "done"; fi

#
# if not optimizing the extraction, then
# move the new temp file to the final destination
# otherwise, process for cleaner extraction
#
if ${nooptimize}
then
    cp ${tmp} ${tmp1}
else
    sort ${tmp} > ${tmp1}
    /bin/rm -f ${tmp}
    if ${veryverbose}; then echo -n "Optimize extraction commands, "; fi
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
    if ${veryverbose}; then echo "done"; fi
fi # no optimize

#
# do the extraction from the archive
#
while read line
do
    if ${veryverbose}; then echo "${line}"; fi
    if ${nodebug}
	(cd ${archive}; ${line})
    fi
done < ${tmp1}

if ${nodebug}
then
    /bin/rm -f $list $tmp $tmp1 ${request}.Work
fi
exec ${mailprog} ${request} ${address} ${processdir} ${prog}
