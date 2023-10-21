#! /bin/csh -f
# This shell script is called from nu to purge one account.
#
# it is named "nu3.sh" instead of something like "killacct.sh" to discourage
# people from trying to run it standalone, without going through nu.
#
# Created:  25 Aug 84	Brian Reid

set exuser=$1
set logindir=$2
set linkdir=$3
set Logfile=$4
set debug=$5

set egrepstr = "^${exuser}\:"
if ($debug == 0) then
    rm -rf $logindir; echo rm -rf $logindir
    if ($logindir != $linkdir) then
	rm $linkdir
	echo rm $linkdir
    endif
    rm -f /usr/spool/mail/$exuser; echo rm -f /usr/spool/mail/$exuser
endif
echo $exuser deleted by $user `date` >> $Logfile
# directories removed; now get them out of /etc/passwd
if ($debug) then
    echo egrep string is $egrepstr
else
    set verbose
    egrep -v $egrepstr /etc/passwd > /usr/adm/nu.temp
    cp /usr/adm/nu.temp /etc/passwd
    rm -f /usr/adm/nu.temp
endif
unset verbose
egrep -s $exuser /usr/lib/aliases
if ($status == 0) then
   echo User $exuser is still referenced in /usr/lib/aliases\; please edit out.
endif
