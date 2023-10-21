#!/bin/csh -f

# PS file made by a mac, uses rf (fortran) type

# This script is intended to be named in the printcap
# as the "rf" filter.  We chose that because it is for
# files of the "Fortran" type (whatever that means) and
# was unused.  Any of the unused printcap filter slots will
# do as well, so long as Mac postscript files are spooled
# with the proper flag to lpr.  We use a shell script called
# maclaser to do that and hide it from the user.

# macfilter is a shell script that figures out which laser-prep file to
# prepend.
# Ron Hitchens	U of Texas CS	(hitchens@sally.utexas.edu)
# Updated April 9, 1987:  Brian H. Powell (brian@sally.utexas.edu)

/usr/local/lib/macfilter | \
	/usr/local/lib/psif -h $host -n $name $pid $acct

if ( $status ) exit 2		# An error, throw the job away
exit 0
