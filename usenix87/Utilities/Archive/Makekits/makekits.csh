#! /bin/csh -f
#
# makekits - generate "kits" from source files for transmission across
#  telephone lines.  Generates an output file called MANIFEST.  That
#  file can be used by later executions of the program as the list of
#  files.
#
#  usage: makekits [ -cMt ] [ -m manifest ]
#				   [ -s size ] -k kitname [ files... ]
#
# note that to do directory traversal, you MUST include the name of
#  the root directory in the list of file names.  In fact, the simplest
#  way to do it is JUST to include the root name as the list of files.
#
# for example, to traverse the current tree and put all regular files
#  in the kits:  "makekits -c -t /usr/local/src/abc"
#
# a short tutorial:
#  first, if there is a file named MANIFEST, remove it
#    if you are using a manifest, and it is named MANIFEST, change it
#  second, DO NOT create your kits in the same directory as those
#    files that are going into the kits, otherwise the kits will be
#    in the kits will be in the kits will...
#
#  change directory to the directory (or root of the directories) containing
#    the files to be placed in kits
#
#  execute the command "makekits -k /tmp/kit *" to pick up only plain
#    files
#
#  execute the command "makekits -t -k /tmp/kit -s 124 *" to pick up all
#    files in the current directory and all subdirectories.  this will
#    cause the creation of a kit numbered zero (0) that will do nothing
#    but create directories.
#
#--------------------------------------------------------------------------
# this script updated 04/18/86 based on a cry for help on the network from
#   Alan Clegg (...!mcnc!ncsu!ncsuvx!abc) to handle directories
#--------------------------------------------------------------------------
#  Steven List @ Benetics Corporation, Mt. View, CA
#  {cdp,engfocus,idi,oliveb,plx,tolerant}!bene!luke!itkin
#--------------------------------------------------------------------------
#
set COMPRESS = cat	# if -c, set to the local compression program
set KITSIZE = 62	# leave room for the shar stuff
set KITNAME = ""	# either from command line or requested below
set MAN_NAME = ""	# may be set from the command line
set MAX_KITS = 20	# limit the number of kits
set TRAVERSE = 0	# if -t, traverse all directory trees found
#
# process command line arguments
#
foreach i ( $* )
	switch ($1)
		case -c:
			set COMPRESS = /usr/lib/news/compress
			set KITSIZE = 100
			shift
			breaksw
		case -k:
			set KITNAME = $2
			shift; shift
			breaksw
		case -m:
			set MAN_NAME = $2
			shift; shift
			breaksw
		case -M:
			set MAN_NAME = MANIFEST
			shift
			breaksw
		case -s:
			set KITSIZE = $2
			shift; shift
			breaksw
		case -t:
			set TRAVERSE = 1
			shift
			breaksw
		case -*:
	echo "usage: makekits [-cMt ][-m manifest][-s size] -k kitname [files...]"
			exit (1)
			breaksw
		default:
			break
			breaksw
	endsw
end
#
if ( "$KITNAME" == "" ) then
	echo "kitname is required"
	echo "usage: makekits [-cMt ][-m manifest][-s size] -k kitname [files...]"
	exit (2)
endif
#
set SIZE = ( 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 )
set FILES = ( ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' ' )
#
switch ("$MAN_NAME")
	case MANIFEST:
		set argv = ( `sed 1,2d MANIFEST | awk '{print $1}'` )
		mv MANIFEST MANIFEST.bu
		breaksw
	case "":
		breaksw
	default:
		set argv = ( `cat $MAN_NAME` )
		breaksw
endsw
#
set FLIST = ( "" )
set DIRS = ( "" )
######################################################################
#
# first, if specified, traverse all directories and add their file
# names to the list of files
#
if ( $TRAVERSE ) then
	foreach file ( $* )
		if ( -d $file ) then
			set DIRS = ( $DIRS `find $file -type d -print` )
		else if ( -r $file ) then
			set FLIST = ( $FLIST $file )
		endif
	end

	set BASE = `pwd`
	foreach dir ( $DIRS )
		cd $dir
		foreach subfile ( * )
			if ( -f $subfile ) set FLIST = ( $FLIST $dir/$subfile )
		end
		cd $BASE
	end
else
	FLIST = ( $* )
endif
			
echo > MANIFEST

foreach file ( $FLIST MANIFEST )
	if ( -d $file ) continue
	set thissize = ( `ls -s $file` )
	set thissize = $thissize[1]
	set kit = 0
	while ( $kit < $MAX_KITS )
		@ kit++
		if ( ( $SIZE[$kit] + $thissize ) <= $KITSIZE ) then
			set FILES[$kit] = "$FILES[$kit] $file"
			@ SIZE[$kit] += $thissize
			echo "$file $kit" >> MANIFEST
			break
		endif
	end
end
#
sort -o MANIFEST MANIFEST
awk '\
BEGIN { print "File Name                 Kit Number"\
		print "--------------            ----------"\
		}\
{ printf "%-24s     %d\n", $1, $2 }' MANIFEST  > tmp$$ 
mv tmp$$ MANIFEST
######################################################################
#
# make a kit to create the directories, if necessary
#
if ( "$DIRS" != "" ) then
	echo "Creating KIT 0 to make directories"
	cat > ${KITNAME}0 << EndHead
#! /bin/sh
# This is a shell archive, meaning:
# 1. Remove everything above the #! /bin/sh line.
# 2. Save the resulting text in a file.
# 3. Execute the file with /bin/sh (not csh) to create the directories:
EndHead
	foreach i ( $DIRS )
		echo "#   $i" >> ${KITNAME}0
	end
	echo "# This archive created: `date`" >> ${KITNAME}0
	echo 'export PATH; PATH=/bin:$PATH' >> ${KITNAME}0
	foreach i ( $DIRS )
		cat >> ${KITNAME}0 << EndDIR
if test ! -d '$i'
then
	echo shar: creating directory "'$i'"
	mkdir '$i'
fi
EndDIR
	end
	echo "#	End of shell archive" >> ${KITNAME}0
	echo "Completed KIT 0"
endif
######################################################################
foreach i ( 1 2 3 4 5 6 7 8 9 10 )
	if ( $SIZE[$i] == 0 ) break
	set NFILES = ( $FILES[$i] )
	echo "Creating KIT $i ($KITNAME$i) - $#NFILES files, $SIZE[$i] blocks"
	shar -p'XX#' -c -v $FILES[$i] | $COMPRESS > $KITNAME$i
	echo "Completed KIT $i ($KITNAME$i)"
end
