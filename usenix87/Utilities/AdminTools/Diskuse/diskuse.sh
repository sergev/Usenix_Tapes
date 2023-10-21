
#! /bin/sh
# Show report of disk usage.  Should be run nightly from cron.  The
# report looks like this:
#	Thu Jun 18 01:12:26 EDT 1987
#	Filesystem    kbytes    used   avail capacity  Mounted on
#	/dev/ra0h      87519   70147    8620    89%    /usr/politics
#	
#	 Today Yester Change   Week Change  Directory
#	 21766  21766      0        +21766  bidden
#	 12645  12645      0        +12645  donaldson
#	 10632  10632      0        +10632  kemp
#	  4493   4493      0         +4493  emacs
#	  2810   2808     +2         +2810  pac
#	  1932   1932      0         +1932  common-cause
#	  1566   1566      0         +1566  guest
#	  1213   1213      0         +1213  root
#	   596    596      0          +596  reagan
#	   594    594      0          +594  bush
#	   349    349      0          +349  hart
#	   248    248      0          +248  kennedy
#	    51     51      0           +51  cuomo
#	 28634      4      0            +4  lost+found
# Where the columns mean:
#	Today		Today's usage
#	Yester		What it was yesterday
#	Change		Difference over the two days
#	Week		What it was last week
#	Change		Difference from last week
#	Directory	Directory within this filesystem.

# Our output goes to stderr; reports go to stdout
exec >&2

# This is where the "raw" du reports go.
USEDIR=/usr/adm/.du
# This is where the daily reports go.
OUTDIR=/usr/adm/disk_use

# Build filesystem list into commands to set shell variables.
LIST=`awk -F: '{ printf "DEV=%s;FS=%s\n", $1, $2 }' </etc/fstab`

for L in ${LIST} ; do
    # Get filesystem and disk device; skip unwanted ones.
    eval ${L}
    if [ "${FS}" = "/" -o "${FS}" = "/tmp" ] ; then
	continue;
    fi

    # Get names for du and pretty reports.
    I=`expr ${FS} : '.*/\(.*\)' '|' ${FS}`
    USE=${USEDIR}/${I}
    OUT=${OUTDIR}/${I}

    # Shift old du reports down...
    for I in 6 5 4 3 2 1 0 ; do
	touch ${USE}.${I}
	mv -f ${USE}.${I} ${USE}.`expr ${I} + 1`
    done

    # Get new du report.
    cd ${FS} ; du -s * | sort +1 >${USE}.0
#   cd ${FS} ; du * | grep -v '\.*/.*'  | sort +1 >${USE}.0

    # Send output to report
    touch ${OUT}
    mv -f ${OUT} ${OUT}.BAK
    exec >${OUT}

    # Prolog
    date
    df ${DEV}
    echo
    awk </dev/null 'BEGIN {
	FMT="%6s %6s %6s %6s %6s  %s\n"
	printf FMT, \
	  "Today", "Yester", "Change", "Week", "Change", "Directory"
    }'

    # Compare today's usage with that of yesterday and a week ago.
    join -j1 2 -j2 2 -a1 ${USE}.0 ${USE}.1 | join -j1 1 -j2 2 -a1 - ${USE}.7 \
    | awk '\
    BEGIN {
	FMT="%6s %6s %6s %6s %6s  %s\n"
    }
    {
	day = $2 - $3
	if (day > 0)
	    dsign = "+"
	else
	    dsign = ""
	week = $2 - $4
	if (week > 0)
	    wsign = "+"
	else
	    wsign = ""
	printf FMT, $2, $3, dsign day, $4, wsign week, $1
    }' | sort -nr

    # Send our output back to stderr.
    exec >&2
done

