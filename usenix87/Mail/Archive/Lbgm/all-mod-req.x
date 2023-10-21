#! SHELL
#
# a script to front end the 'get-mod-req' program
# Automatically finds all requests with name 'REQ*'
# for procesing.  Calls '${getmodreq}' for each request
#
#
# see get-mod-req for what to change below
#
process=PROCESS	# directory which we work in.

getmodreq=${process}/get-mod-req

if [ ! -f ${getmodreq} ]
then
    echo "Can't find '${getmodreq}'" 1>&2
    exit 1
fi

if [ $# != 0 -a $# != 2 ]
then
    echo "Usage: `basename ${prog}` [ Mail whom ]" 1>&2
    exit
elif [ $# = 2 -a "$1" != "Mail" ]
then
    echo "Usage: `basename ${prog}` [ Mail whom ]" 1>&2
    exit
elif [ $# = 2 ]
then
    request=$1
    address=$2
    exec ${getmodreq} ${processdir} ${request} ${address}
else
    request=`ls REQ*`
    if [ x"${request}" = "x" ]
    then
	echo "No REQ files waiting to be sent" 1>&2
	exit 1
    fi

    for req in ${request}
    do
	address=`echo ${req} | sed -e 's/REQ\.//'`
	${getmodreq} ${process} ${req} ${address}
	if [ $? != 0 ]
	then
	    exit
	fi
    done
fi
