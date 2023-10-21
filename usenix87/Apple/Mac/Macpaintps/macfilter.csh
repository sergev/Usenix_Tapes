#!/bin/csh -f
#	macfilter
#  Filter for Macintosh-generated PostScript code.
#  By Ron Hitchens & Brian Powell
#	hitchens@sally.utexas.edu	brian@sally.utexas.edu
#	hitchens@ut-sally.UUCP		brian@ut-sally.UUCP
#
#   This filter prepends the necessary LaserPrep file to the Mac file, then
# sends it through a filter to escape 8-bit characters (otherwise lost by the
# UNIX printer driver.)  This version supports the Macintosh LaserWriter driver
# versions 1.1 (LaserPrep version 12), 3.1 (LaserPrep version 40), and 3.3
# (LaserPrep version 49).
#   The difference between the three versions is deduced by looking at the
# first line of the input to this script.  The first line from the 1.1 driver
# consists entirely of "md begin".  The first line from the 3.1 driver consists
# of "%!PS-Adobe-1.0".  The first line from the 3.3 driver consists of
# "%!PS-Adobe-1.2".  This script fgreps for "Adobe-1.2" in the first line and
# if found, prepends the version 49 prep.  Then it fgreps for "Adobe-1.0" and
# if found, prepends the version 40 prep.  Otherwise, it prepends the
# version 12 prep.
#   NOTE:  Other versions of the LaserWriter driver (most notably version 3.0)
# produce output that is not easily distinguishable from output from Laser-
# Writer driver 3.1.  For this reason, this filter cannot provide warnings
# about incorrect input.  In general, the different versions are incompatible,
# and correct output from anything other than PostScript from the Macintosh
# LaserWriter drivers 1.1, 3.1 and 3.3 cannot be expected.


set prepdir=/usr/local/lib/allfonts/postscript	# dir where the prep files live

cat > /tmp/mac$$			# save stdin so we can look at it

# search the first line of stdin for "Adobe-1.2" and "Adobe-1.0".  The
# variable stat1 is true if "Adobe-1.2" isn't found, and the variable stat2
# is true if "Adobe-1.0" isn't found.

head -1 /tmp/mac$$ | fgrep -s Adobe-1.2 >& /dev/null
set stat1=$status
head -1 /tmp/mac$$ | fgrep -s Adobe-1.0 >& /dev/null
set stat2=$status

if ( ! $stat1 ) then
	set prep=laser-prep-49.pro	# found "Adobe-1.2"; use version 49
else if ( ! $stat2 ) then
	set prep=laser-prep-40.pro	# found "Adobe-1.0"; use version 40
else
	set prep=laser-prep-12.pro	# not found; assume version 12
endif

# Concatenate the prep and the Mac job.  The combined Postscript is then
# piped thru a filter to escape any chars with the high bit set (Apple
# dropped the ball on that one).  The final result goes down our stdout
# which is usually being piped into the printer driver (psif or pscomm)

cat $prepdir/$prep /tmp/mac$$ | /usr/local/lib/ps8

set result=$status		# save the result for our exit code

rm -f /tmp/mac$$		# make sure rm runs silently

exit $result			# that all
