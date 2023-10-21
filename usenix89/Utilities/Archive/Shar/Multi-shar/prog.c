
#! /bin/csh
#
# makekits - generate "kits" from source files for transmission across
#  telephone lines.  Generates an output file called MANIFEST.  That
#  file can be used by later executions of the program as the list of
#  files.
#
#  usage: makekits [ -cM ] [ -m manifest ]
#				   [ -s size ] -k kitname [ files... ]
#
set COMPRESS = cat	# if -c, set to the local compression program
set KITSIZE = 62	# leave room for the shar stuff
set KITNAME = ""	# either from command line or requested below
set MAN_NAME = ""	# may be set from the command line
set MAX_KITS = 20	# limit the number of kits
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
		case -*:
	echo "usage: makekits [-cM ][-m manifest][-s size] -k kitname [files...]"
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
	echo "usage: makekits [-cM ][-m manifest][-s size] -k kitname [files...]"
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
echo > MANIFEST
foreach file ( $* MANIFEST )
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
#
foreach i ( 1 2 3 4 5 6 7 8 9 10 )
	if ( $SIZE[$i] == 0 ) break
	shar -c -v $FILES[$i] | $COMPRESS > $KITNAME$i
end
-- 
UUCP: ..!{allegra,decvax,seismo}!rochester!ken ARPA: ken@rochester.arpa
Snail: CS Dept., U. of Roch., NY 14627. Voice: Ken!
