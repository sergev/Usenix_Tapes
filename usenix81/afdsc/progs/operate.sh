/bin/echo -en "\n\n***********  OPERATE  ROUTINE  **********\n\n"
/bin/who|/bin/wc|/bin/arfi -a 1>/tmp/oper$$
set c : /tmp/oper$$
/bin/rm /tmp/oper$$
/bin/if $c = 1 /bin/goto alone
set c - 1
/bin/echo "There are "$c" other users currently logged onto the "$S" UNIX system.\nPlease use "wall", "drain", and "kickoff" to clear the system."
/bin/who
/bin/exit
: alone
/bin/echo "The "$S" UNIX system is clear of users.  You may proceed."
/bin/echo -en "\n\nDID YOU JUST REBOOT? "
/bin/if ! { /bin/query -c } /bin/goto MAIN
/bin/drain "Unix File Saves Are In Progress." 2>/dev/null
set s = "AFTER"
:
: Do this section AFTER REBOOT
:
: set time
/bin/echo -n "\nENTER TODAY'S DATE (Example: 8 Feb 80): "
set d : $T
/bin/echo -n "ENTER CURRENT TIME (24-Hour Style): "
set t : $T
/bin/echo -en "\nCURRENT DATE AND TIME ARE "
/bin/echo $d"\n"$t | /bin/date -i -u ^ /tmp/oper$$
set t =
set d =
set t : /tmp/oper$$
set d = $t
/bin/if x$d = x /bin/goto mount
/bin/echo "\nThe date or time was entered incorrectly.  Try again."
/bin/goto set
:
: mount files
/bin/rm /tmp/oper$$ ^ /dev/null
/bin/fsmount
:
/bin/up -u
: Start up cron -- Must be done AFTER fsmount
/bin/nohup /sys/prog/cron
: Start up probe also
/sys/prog/probe -f -c 180 &
:
: MAIN BODY OF ROUTINE -- Done both Before and After BOOT
:
: do the file checks, dumps, and print maglib
onintr -
:
: file checks
:
/bin/echo -en "\n\nDO YOU WANT FILE CHECKS, Y or N? "
/bin/if ! { /bin/query -c } /bin/goto endfc
/bin/echo "File Checks are being generated.\nThey will be sent to the System Administrator via the UNIX mail system."
(/bin/echo "\nsys.admin:\n\n"$S" UNIX File System Consistency Checks";/bin/fschecks;/bin/echo "s") | /bin/sndmsg > /dev/null
:
: dump files
: endfc
/bin/fssaver < $T
onintr
/bin/if $sx = AFTERx /bin/goto finale
:
: do this section BEFORE REBOOT
:
: reboot
/bin/echo -en "\nARE YOU PLANNING TO REBOOT THE SYSTEM? "
/bin/if ! { /bin/query -c } /bin/goto finale
/bin/kickoff . .>/dev/null
:
: kill cron
/bin/ps -x | /bin/grep "[0123456789] /sys/prog/cron" | /bin/arfi -a 2 | set k :
/bin/kill $k
: Get probe to kill itself
/sys/prog/probe -f -k
:
: unmount all files
/bin/fsumount ^/dev/null
/bin/echo -e "\n\nALL FILES ARE UNMOUNTED"
/bin/mount >/sys/tmp/EOD.2
/bin/if { cmp /sys/tmp/EOD.2 /dev/null >/dev/null } /bin/goto endumnt
/bin/echo -e "  except the following"
/bin/sort /sys/tmp/EOD.2
: endumnt
/bin/rm /sys/tmp/EOD.2
/bin/sync
exit
:
: finale
/bin/echo -e "\n\nMOUNTED CONFIGURATION\nDRIVE  FILSYS\n"
/bin/mount -a | /bin/sort
/bin/echo -en "\n\n\nDATE AND TIME ARE "
/bin/date
/bin/leton
/bin/sync
exit

Name:
	OPERATE User Command

Function:
	Allow operators to perform operational functions for UNIX.
	This is an interactive routine that allows the user to select
	what operational tasks he/she wants performed, such as:
		mount file systems after a reboot
		set system time after a reboot
		print out the Status of the file systems
			(ala icheck and dcheck)
		dump any file systems
		print the maglib listing
		unmount the files and ready the system for a reboot

Algorithm:
	OPERATE is mostly a framework that calls other shell files to do all
	the work.  If you just rebooted, it will mount the files,
	set the system date, and kick cron.  FSMOUNT is called to mount the
	files.  FSSAVER is called to do the file checks, the dumps, and to print
	the maglib.  A DRAIN is done at the starting to lock out the system,
	and if you answer YES to any of the questions, it will do a KICKOFF
	to clear the system of users.  FSUMOUNT is called to unmount the files
	if a reboot is desired, and SYNC is executed just to make sure everything
	is "in sync" before you hit the 'ol Halt key.

	SHELL VARIABLES
	s	= "AFTER" if a reboot was just done
	t	the 4 digit current time, as input by user
	d	the numeric day (2 digit)
	x	the numeric month (2 digit)
	z	the process id of cron

Parameters:
	NONE

Returns:
	YES

Files and Programs:
	/bin/drain		A shell file to lock out the system.
	/bin/ps, /bin/kill	To "kick cron".
	/sys/prog/probe		To capture performance data.
	/bin/fssaver		A shell file that prompts for filchecks,
				dumps, and prints the maglib listing.
	/bin/fsmount		A shell file that mounts all the file systems.
	/bin/kickoff		A shell file that kicks all users off the system.
	/bin/mount, /bin/sort	To print out the mounted files at the end.
	/bin/sync		To get the file systems in sync with core.
	/bin/date		To set and print the system date and time.
	/bin/leton		A shell file to let users back onto the system.

Installation Instructions:
	chmod 750 operate		only root and operators should have access
	mv operate /bin/operate

History:
				Written by Dan Chernikoff, AFDSC/SFA
	9 May 79		Documentation added for The Great Command Rewrite.

~|^' operate  Release 3 Version 0
