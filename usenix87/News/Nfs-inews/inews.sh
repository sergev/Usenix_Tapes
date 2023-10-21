#!/bin/sh
# A common situation is for a machine to use NFS to mount /usr/spool/news
# remotely from another machine. This lets you read news via rn,
# but it does not let you post news, since you have no inews program for
# Pnews to call.
# 
# This shell will let you post mail by sendmailing it to another machine.
# It was written to run on Suns, but can probably be made to work on other machines.
#
# You will have to change the name "oopsvax" to the name of a machine you
# can send mail to that has rnews. (We use shasta, but you shouldn't unless you
# are at Stanford).
#
# You will need to create the file /usr/lib/news/counter
# (or wherever you decide to put it) to count message numbers.
# It should start out containing the number "1".
#
# A copy of your article will be left in your home directory, called "article".
#
# By Joe Dellinger
# further hacking by Conor Rafferty
#

#initialization
counter=/usr/lib/news/counter
userart=$HOME/article
oopsvax=???????
itmp=/tmp/itmp$$
trap "rm -f /tmp/itmp*" 0 1 2 3 13 15

#host and user name. Try the password file, then the yellow pages muck.
if test "$hostname" = "" ; then hostname=`hostname`; fi
passline=`grep :$USER: /etc/passwd` ||
passline=`ypmatch $USER passwd.byname` ||
passline=dummy:dummy:dummy:dummy:$USER
export passline
fullname="`(IFS=:;set $passline; echo $5)`"

# Grab stdin
cat > $userart
if \[ -f $HOME/.signature \] ; then
cat $HOME/.signature >> $userart
fi

# Increment the message ID number for this system
number=`cat $counter`
number=`expr $number + 1`
echo $number > $counter

# simulate inews header
# remote rnews does a lot of the work itself (date reformat, Lines)
echo "Subject: network news posting"            > $itmp
echo "To: rnews@$oopsvax"                      >> $itmp
echo ""                                        >> $itmp
echo "NPath: $hostname!$USER"                  >> $itmp  #path back to you
echo "NFrom: $USER@$hostname.UUCP ($fullname)" >> $itmp  #want the full name
echo "NMessage-ID: <$number@$hostname.UUCP>"   >> $itmp  #rnews junks it otherwise
echo "NDate: `date`"                           >> $itmp  #ditto

#stuff in the user input
sed 's/^/N/' $userart                          >> $itmp


# Send it off to $oopsvax to be posted
/usr/lib/sendmail -i rnews@$oopsvax < $itmp

# That's it. The article is left in your home directory.
