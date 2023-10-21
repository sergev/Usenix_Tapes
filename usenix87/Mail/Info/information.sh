:
#!/bin/sh
#
#  Return a mail message to the sender containing the contents of the
#  information file requested.  The message header is altered mainly to allow
#  error messages to be returned to the maintainer (and the original sender).
#  This script should be setuid to one of sendmail's trusted users.
#
#  Usage: put an entry into /usr/lib/aliases of the following form:
#
#	information: "|/usr/local/lib/information"
#

mysite=uk.ac.my.site
helpdir=/information
logfile=/usr/adm/log/information

rm -f /tmp/info.$$

sed -e '/^$/,$d' > /tmp/info.$$

subject=`sed -n -e '/^Subject: */s///p' < /tmp/info.$$ | tr '[A-Z/]' '[a-z!]'`
to=`sed -n -e '/^From:.*<\(.*\)>.*/s//\1/p' -e '/^From:\(.*\)(.*/s//\1/p' < /tmp/info.$$`

cat > /tmp/info.$$ << END
From: Information Service <information-request@$mysite>
To: $to
Subject: Re: $subject

	Thank you for your request for information.  Requests for information
should be sent to "information@$mysite", with a "Subject:" header line
containing the information requested.  A "Subject:" header line containing the
word "index" may be used to get a list of possibilities.  Comments about this
program should be sent to "information-request@$mysite".
END

if [ -f "$helpdir/$subject" ]
then
	echo GOOD: "$subject": "$to": `date` >> $logfile
else
	echo BAD: "$subject": "$to": `date` >> $logfile
	cat >> /tmp/info.$$ << END

I couldn't find any information about "$subject".
Here is the index instead:
END
	subject=index
fi

cat >> /tmp/info.$$ << END

`cat "$helpdir/$subject"`
END

if /usr/lib/sendmail -t < /tmp/info.$$
then
	status=0
else
	status=69
fi

rm -f /tmp/info.$$

exit $status
