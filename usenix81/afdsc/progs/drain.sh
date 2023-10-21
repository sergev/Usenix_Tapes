set n = 100
/bin/cp /usr/sys/ttysall /sys/ttys
/bin/cp /usr/sys/passwd2 /usr/sys/passwd
: Tell probe system is down
/sys/prog/probe -f -d
/bin/if $N -l 3 /bin/goto needtime
set t = $2
/bin/if $2x = .x /bin/goto sametime
/bin/if $2 -l 100 /bin/goto setinc
: newmsg
/bin/if $1x = .x /bin/goto newtimer > /dev/null
/bin/echo $1"\nWill be back up at "$t >/sys/msg/unavail
/bin/cat /sys/msg/unavail
exit
: setinc
set n = $200
: needtime
(/bin/date | /bin/rpl ":" "" | /bin/arfi -a 4 -p 1,4; /bin/echo "+" $n) | /bin/bc > /sys/tmp/drain.foo
set t : /sys/tmp/drain.foo
/bin/if $N != 1 /bin/goto newmsg
: default
/bin/echo "Unix file maintenance is in progress,\nWill be back up at "$t > /sys/msg/unavail
/bin/cat /sys/msg/unavail
exit
: newtimer
/bin/dog /sys/msg/unavail > /sys/tmp/drain.foo
/bin/mv /sys/tmp/drain.foo /sys/msg/unavail
/bin/echo "Will be back up at "$t >> /sys/msg/unavail
/bin/cat /sys/msg/unavail
/bin/rm /sys/tmp/drain.foo* ^ /dev/null
exit
: sametime
/bin/dog -l 2 /sys/msg/unavail > /sys/tmp/drain.foo
/bin/arfi -a 6 < /sys/tmp/drain.foo > /sys/tmp/drain.fooa
set t : /sys/tmp/drain.fooa
/bin/rm /sys/tmp/drain.foo* ^ /dev/null
/bin/if $N != 1 /bin/goto newmsg
/bin/goto default

Name:
	DRAIN System Command

Function:
	To prevent any more users from loggin onto UNIX, and to provide
	an explanatory message why they are shut out.

Algorithm:
	Copy /usr/sys/passwd2 to /usr/sys/passwd to lock users out.  Then supply an appropriate
	message in /sys/msg/unavail.  This is done as follows:
	If no time argument was provided, determine the current time
	(using DATE, RPL, and ARFI).
	If no message argument was provided, use a canned default message.
	If a time was provided, and it is less than 100, it is a time
	increment.  Use DATE, RPL, ARFI, and BC to compute the current
	time plus the supplied increment.
	If a time argument >100 was provided, use it as the time.
	If a message was provided, use it.

	SHELL VARIABLES
	n	The time increment to be added to the current time.
	t	The time to be used in the unavail message

Parameters:
	drain message time

	message		Text for the unavail message.  If it is ".", then use
			the current message.  If it is null, then use a default
			message.
	time		The time UNIX will be available.  It can either be the
			actual time, or a time increment (in hours).  If it is
			".", then the time last used is preserved.  If it is null,
			use the current time plus 1 hour.

Returns:
	YES

Files and Programs:
	/bin/arfi	To extract arguments from the date.
	/bin/date	To determine the current time.
	/bin/rpl	To finagle the output of date.
	/sys/prog/probe	To signal real probe system is down.
	/bin/bc		To add the increment to the current time.

	/sys/msg/unavail	The file containing the unavailable message which is
				printed when a user tries to log in.
	/usr/sys//usr/sys/passwd		The password file that is referenced when a user logs in.
	/usr/sys//usr/sys/passwd2	A modified password file that replaces the shell with
				a program to print out the unavail message.

Installation Instructions:
	mv drain /bin/drain
	chmod 750 /bin/drain	Root and operators only should use it.

History:
	14 May 79	Documentation added for The Great Command Rewrite.

~|^' drain  Release 3 Version 0
