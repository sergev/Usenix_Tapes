
#! /bin/sh
#
# cn - print available news in multi-column format.
#
# Adapted from a shell script originally written by:
# Sun Nov 30 22:34:48 EST 1986	(...!sunybcs!gworek / ...!boulder!forys)
# Modified:
# Mon Dec 15 08:30:05 PST 1986
# by
# Brian L. Matthews, ...uw-beaver!ssc-vax!cxsea!blm
#

GROUPS=30

while [ $# != 0 ]
do
    case $1 in
	-v)	vertical=1
		;;
	[0-9]*)	GROUPS=$1
		;;
	*)	echo usage: $0 [ -v ] [ groups ]
		exit 1
		;;
    esac
    shift
done

if [ -n "$vertical" ]
then
    rn -s$GROUPS -c | awk '
    /^etc.$/{
		etc = 1;
		exit;
	    }
	    {
		group[NR] = $4;
		count[NR] = $5;
	    }
    END	    {
		groups = NR - etc;
		rows = int ((groups + 2) / 3);
		for (i = 1; i <= rows; i++)
		{
		    for (j = i; j <= groups; j += rows)
			printf ("%3d %-21.21s ", count[j], group[j]);
		    print "";
		}
		if (etc)
		    print "etc...";
	    }'
else
    rn -s$GROUPS -c | awk '
    /^etc.$/{
		etc = 1;
		exit;
	    }
	    {
		printf ("%3d %-21.21s ", $5, $4);
		if (NR % 3 == 0)
		    print "";
	    }
    END	    {
		if (NR && NR % 3)
		    print "";
		if (etc)
		    print "etc...";
	    }'
fi

exit 0
