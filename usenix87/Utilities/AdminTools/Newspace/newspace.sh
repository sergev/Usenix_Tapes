
# print filespace used by each newsgroup, largest-first

active=/usr/lib/news/active
newsdir=/usr/spool/news

# make sure needed files/directories are available

if [ ! -f $active ]
then
	echo "$0: can't find active file" 1>&2
	exit 1
fi

if [ ! -r $active ]
then
	echo "$0: can't open active file" 1>&2
fi

if [ ! -d $newsdir ]
then
	echo "$0: no $newdir directory!" 1>&2
	exit 1
fi

cd $newsdir

# the first section of the pipeline meaures the filespace under each
#       directory which corresponds to a newsgroup name in the
#       active file

# the following cat-while loop is supplied in case you dont have cut(1);
#       it's output should be piped into xargs
	
#cat $active | while read ng junk	# fetch newsgroups
#do
#	echo $ng
#done |

# another choice: John Nelson (panda!jpn) supplied the following
#       alternative for the cut-tr-xargs-du pipeline; its output
#       should be piped into awk for the large awkscript below

#du -s `awk '{ print $1 }' $active | tr '.' '/'` |

# OK, here's the vanilla System V version ...

cut -d" " -f1 -s $active |              # substitutions for this supplied above
	tr '.' '/' |			# cvt newsgroup names to dirs
	xargs du -s |			# measure them all

# this awk program takes these measurements of the
# directories and redistributes the weights of specific newsgroups;
# that is, the size of net/music/gdead is subtracted from the
# the size of net/music, and so on.

awk '
	{ newsgroups[$2] = $1 }
END {
	for (ng in newsgroups) {
		n = split(ng, l, "/")
		tng = l[1]		# parent newsgroups of ng

		# look at each parent (?ancestor) of this newsgroup

		for (i = 2; i <= n; i++) {

			# subtract total of this newsgroup from total
			#	of parent newsgroups

			if (newsgroups[tng] > 0)
				newsgroups[tng] -= newsgroups[ng]
			tng = tng "/" l[i]
		}
	}

	# print them out (only non-empty ones, tab-separated)

	for (ng in newsgroups) {
		if (newsgroups[ng] > 0)
			print newsgroups[ng], "\t", ng
	}
}' |
	tr '/' '.'  |		# restore standard newsgroup names
	sort +0nr		# largest newsgroups first
