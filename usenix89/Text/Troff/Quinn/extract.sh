#! /bin/sh
# extract -- print out troff macro definitions
# author -- R E Quin, Warwick University (UK) 1983
# (version 2 November 1985)
# see endnotes
soelim=/usr/ucb/soelim
# mkpat puts a \ in front of sed/grep meta-chars in its args
mkpat=/usr/local/usr/lib/troff/mkpat

PATH=/bin:/usr/bin:/usr/ucb:/usr/local/bin
mac_file=/usr/lib/tmac/tmac.s    # default macro package is -ms

for i in "$@"
do
	case x"$i" in

	x-m*)
		mac_file=/usr/lib/tmac/tmac.`echo $i |sed -e 's/-m//'`
		;;
	x-f*)
		mac_file=`echo $i | sed -e 's/-f//'`
		;;
	x*)
		pat=`${mkpat} "$i"`
		${soelim} ${mac_file} | sed -n -e \
		    '/^[.'"'"'][ 	]*d[es][ 	]*'"${pat}"'$/,/^\.\./p
		     /^[.'"'"'][ 	]*d[es][ 	]*'"${pat}"'[ 	]/,/^\.\./p'
		;;
	esac
done
exit

# bugs:
# gets nested macro definitions wrong (will print out from the start of the
# definition of the inner macro right up to the end of the enclosing one. It
# would be hard to deal with that one.
#
# can't cope with .cc, so if you .de doesn't start with a ".", tough.
#
# won't get .de xx yy right -- always looks for .. at end.
# 
# most of the problems involve getting too much text out though.
#
# soelim can't cope with .ds xx file .... .so \*(xx, so it fails on the
# 4.2 ms macros.  There is no way to deal with this, since some of the macros
# in the .so files overwrite ones elsewhere!
#
