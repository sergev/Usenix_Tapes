:
if [ ! $# = 0 ]
then	logins=$*
else	logins=`cut -f1 -d: < /etc/passwd`
fi

uroot=`dirname $HOME`
mroot=`dirname $MAIL`
users=

for i in $logins
do	if [ -d $uroot/$i ]
	then	user="$user $i"
	fi
done

while :
do	for i in $user
	do	if [ ! -s $mroot/$i ]
		then	quote | mail $i
		fi
	done
	sleep 5
done
