/bin/cp /usr/sys/passwd1 /usr/sys/passwd
/bin/cp /usr/sys/ttysall /sys/ttys
/bin/kill -s 1 1
: Tell probe system is up
/sys/prog/probe -f -u
exit

Name:
	LETON System Command

Function:
	To let users back onto the system after a DRAIN or KICKOFF.

Algorithm:
	Copy the passwd1 file into passwd to change users back to the default
	shell (instead of the unavail program) when they log in.
	Copy ttysall into ttys to make sure we have the standard system tty
	configuration.  Send a hangup to init to make it look at the ttys file.

Parameters:
	NONE

Returns:
	YES

Files and Programs:
	/usr/sys/passwd		The login passwd file.
	/usr/sys/passwd1	A copy of the passwd file that has sh as the shell.

	/bin/kill		To kick init in the pants.
	/sys/prog/probe		To signal real probe system is up.

Installation Instructions:
	mv leton /bin/leton
	chmod 750 /bin/leton	Only root and operators need use.

History:
	14 May 79	Documentation added for The Great Command Rewrite.

~|^' leton  Release 3 Version 0
