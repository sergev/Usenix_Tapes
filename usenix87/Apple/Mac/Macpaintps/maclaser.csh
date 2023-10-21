#! /bin/csh -f

# A simple script to spool files prepared on a Macintosh for printing on
# the LaserWriter.  It expilictly spools to the lw printer and flags the
# file as "Fortran" type.  This file type is redefined locally to mean
# a mac Postscript file, causing the Apple laser-prep file to be prepended
# to provide the standard Mac environment in the printer when the file
# is processed.
# Files spooled with maclaser should have been created on the Mac with
# the COMMAND-F sequence and the resulting PostScript file uploaded  [RH]
#
#  April 1986	Generalized to handle arbitrary option args and accept
#		multiple file names.  This is now a general filter for
#		calling lpr.  Options and file names can be mixed up in
#		the "wrong" order and they will be given to lpr in the 
#		order it wants them.  The 'always' variable is what makes
#		this a script specific to handling Mac Postscript.
#		Ron Hitchens (hitchens@sally), Brian Powell (brian@sally)
#

set lpr = /usr/ucb/lpr
set lw = -Plw
set always = (-f)		# this means Mac PostScript (local spooler hack)
set flags = ()
set files = ()

top:
	if ($#argv > 0) then
		switch ($argv[1])
		case -P*:		# need to look for this one specifically
			set lw=$argv[1]	# since we've setup a default
			shift argv
			goto top
		case -:			# pretend this means read stdin
			shift argv	# lpr reads stdin if no files names
			goto top	# are given anyway.  Ignore it.
		case -*:
			set flags = ($flags $argv[1])
			shift argv
			goto top
		default:
			set files = ($files $argv[1])
			shift argv
			goto top
		endsw
	endif

exec $lpr $always $lw $flags $files
