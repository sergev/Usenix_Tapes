#! SHELL
# Add-bugs
#  Build the list of new bugs for which we dont already
#  have entries.
#
# EGREP could be bm or egrep
GREP=egrep
t1=/tmp/add-bug-f1.$$
t2=/tmp/add-bug-f2.$$
bugs=bugfix-list
trap "/bin/rm -f ${t1} ${t2} ${bugs}; exit" 0 1 2 3 9 15
if [ ! -f TOCFROMSOURCEARCHIVE ]
then
    echo "Missing Total table of contents 'TOCFROMSOURCEARCHIVE'" 1>&2
    exit 1
fi

if [ ! -f quicklist ]
then
    echo "Missing quick lookup list 'quicklist'" 1>&2
    exit 1
fi
#
# build sorted list of unique names to search for
#
grep -i bug TOCFROMSOURCEARCHIVE > ${t1} &
grep -i fix TOCFROMSOURCEARCHIVE > ${t2}
wait
sort -u ${t1} ${t2} > ${bugs}
/bin/rm ${t1} ${t2}
#
# read the name in, and sort for it, looking for a 'bug or fix'
#
awk '{print $1}' < quicklist | sort -u | while read line
do
    grep -i ${line} ${bugs}
done |
#
# lookup the date in the quicklist, and save only those
# which are not already there
#
awk -F: '{print $1}' | while read date
do
    ndate=`echo ${date} | sed -e 's/85_//' -e 's/86_//' -e 's/84_//'`
    ${GREP} -s ${ndate} quicklist
    if [ $? = 1 ]
    then
	#
	# find the entire line for which the bug/fix was
	# found, leaving it in a file called NEWBUGS
	#
	${GREP} ${date} TOCFROMSOURCEARCHIVE >> NEWBUGS
    fi
done
exit 0
