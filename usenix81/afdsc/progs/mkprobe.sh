/bin/date
/bin/if $1x = x /bin/goto error
: loop
/bin/echo Making $1
/bin/cp /usr/include/$1.sid.h /usr/include/sid.h
/usr/bin/cc probe.c
/bin/chk -c probe
/bin/mv probe /sys/prog/generate/$1/probe
/bin/if $1x != wp0x /bin/goto cont
/bin/rm /sys/prog/probe
/bin/ln /sys/prog/generate/$1/probe /sys/prog/probe
: cont
shift
if $1 != x /bin/goto loop
/bin/echo "mkprobe finished"
/bin/date
/bin/exit
: error
/bin/echo "Usage: mkprobe sys1 sys2..."
/bin/exit
/*

Name:
	mkprobe

char *prog_id "%Z%%M% Release %R% Level %L% Branch %B% Sequence %S% %D% %T%\n";
Function:
	make probe software for each system configuration.

Algorithm:
	put proper site id (sid.h) information into place
	in /usr/include, compile and place output
	in appropriate /sys/prog/geneate/system/probe
	file.

Parameters:
	mkprobe wp1 wp2
		to do only wp1 and wp2

Returns:
	nothing

Globals:

Calls:

Called by:
	used only for installing probe after a change
	in kernels or a change in probe.

Files and Programs:
	/usr/include/wpx.sid.h - where x is the system number.
	/usr/include/sid.h - actual site id.
	/sys/prog/generate/wpx/probe - location of probe
	/sys/prog/probe - location of probe for wp0 and
		on any running system.

Notes and Memorabilia:

Installation Instructions:
		run this program after any change to
		sid.h files for the system involved.

History:
	Written by Charles Muir, Nov 1980.


*/
