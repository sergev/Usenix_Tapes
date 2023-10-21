#! SHELL
# Build the file 'quicklist' (TESTOUTPUT), for use by the programs
# 'all-quick-req' and 'quick-mod-req'
#
newdata=NEWVOLS
datalist=TESTOUTPUT

while read line
do
    set $line
    if [ $1 = "VOLUME" ]
    then
	volume=$2
	echo "Processing Volume '${volume}'"
    else
	issues=$1
	shift
	name=$1
	shift
	title="$*"
	set `echo ${issues} | sed -e 's/-/ /'`
	if [ $# != 1 ]
	then
	    count=$1
	    limit=$2
	else
	    count=$1
	    limit=$1
	fi
	while [ ${count} -le ${limit} ]
	do
	    filename=`grep "Volume ${volume}, Issue ${count}$" VOLLIST |\
		    awk '{ print $1 }' | sed -e 's,/, ,' -e 's,:,,'`
	    echo "${name} ${filename} ${title}" >> ${datalist}
	    count=`expr ${count} + 1`
	done
    fi
done < ${newdata}
