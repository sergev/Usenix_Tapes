#! /bin/csh -f
# checkin  -- prompt user to create log files, then do `ci -l` for the
#	      files that have a log.
#
# 03/85 mhfoster, ogc
#
# usage:
#	checkin [file1 file2 ...]
#
# For each file specified on the command line:
#
# If there was a difference in the checked-out version
# and the latest checked-in version OR the rcsdiff failed,
# put the user into their editor, editing the diff (or fail 
# message).  This allows a user to peruse the diff, and create 
# a comment based on such.  (Alternatively, a separate message 
# could be defined and included while in the editor.)
#
# Once a group of log files has been created, the source files are
# checked-in using the defined log files.
#
# Notes:
#  If environment variable CHECKIN is set, it specifies `ci` options;
#  default is -l for "check back out locked".
#  If environment variable EDITOR is set, it specifies the editor to use;
#  default is `vi`.
#
onintr quiter

set cmd = $0
set cmd = $cmd:t
if ($#argv < 1) then
	echo "usage: $cmd file1 file2 ..."
	exit
endif

set cilist = ""
set tmpdir = "/tmp"
set versfile = $tmpdir/CI.v.$$
set diffile = $tmpdir/CI.d.$$
if ($?EDITOR) then
	set editor = $EDITOR
else
	set editor = "vi"
endif
if ($?CHECKIN) then
	set ciopts = "$CHECKIN"
else
	set ciopts = "-l"
endif

foreach f ($*)
	if (-d "$f") then
		echo "Cannot checkin: $f  is a directory"
	else
		echo "Checking for differences in $f"
		set tmplogf = $tmpdir/CI.d.${f:t}.$$
		set loclogf = _log.${f:t}.$$
		(rcsdiff $f > $diffile) |& tee $versfile

		set diffstat = "$status"
		if ( !(-z $diffile) || ($diffstat)) then
			cat $versfile $diffile > $tmplogf
			if ($diffstat) then
echo "Enter Description (this is NOT the log message)" >> $tmplogf
			endif
			$editor $tmplogf
			echo -n "Want to checkin $f with the log? (y) "
			set r = "$<"
			if (($r =~ "") || ($r =~ y*)) then
				mv $tmplogf $loclogf
				set cilist = "$cilist $f"
			endif
		endif
	endif
end
if ("$cilist" != "") then
	echo " "
	echo "Performing checkin of:"
	echo "$cilist"
	echo " "
endif
# this portion is separated from the above loop so the user doesn't have
# to interactively wait for the actual rcs checkin (ci) process to finish
foreach f ($cilist)
	set loclogf = _log.${f:t}.$$
	if (! (-z $f)) then
		ci $ciopts $f < $loclogf
		if (! $status) then
			rm $loclogf
		else
			"$cmd, ci error, log saved: $loclogf"
		endif
	else
		"$cmd error: $f does not exist (cannot checkin)"
	endif
end
rm $tmpdir/CI*.$$
exit

quiter:
echo "$cmd, Abort."
rm $tmpdir/CI*.$$
exit
