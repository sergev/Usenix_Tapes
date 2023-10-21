#! SHELL
# rmarch:
#	Extract a Message-Id from a news file and prepend an lbgmX.
#	Used to help keep lbgm archives clean.
#	Outputs a file MYHOME/tozap
#

# extract Message-Id
# $1 = article number
# $2 = Newsgroup

trap "/bin/rm -f /tmp/rm$$; exit 0" 1 2 3 4 5 6 7 8 9

head -12 > /tmp/rm$$

echo "echo 'Deleting: $1 in newsgroup $2'" >> MYHOME/tozap
awkres=`awk  'BEGIN { sub = ""; mid = "" ; FS = ":" }
/^Subject:/ {
	for (i = 2; i <= NF; i++)
		sub = sprintf("%s %s", sub, $i);
	printf("%s", sub);
	}' < /tmp/rm$$`

subject=`echo ${awkres} | tr "'" " "`
echo "echo 'Deleting:${subject}'" >> MYHOME/tozap

awk  'BEGIN { sub = ""; mid = "" ; FS = ":" }
/^[Mm][eE][sS][sS][aA][gG][eE]-[Ii][Dd]:/ {
		if ( mid == "" ) mid = $2;
		printf("DELPROG \"%s\"\n", mid);
		exit
	}' < /tmp/rm$$ >> MYHOME/tozap

chmod +x MYHOME/tozap
/bin/rm -f /tmp/rm$$
