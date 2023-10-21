#! SHELL
# ckarch: check that a given article exists
#
# Extract a Message-Id from a news file and process to check for
# inclusion in the archive.  add code to check for missing entries
# to be added automatically
# used to help keep lbgm archives clean
# called from rn with " | ckarch article-num Newsgroup "
# outputs a file MYHOME/tozap
# archive can then be cleaned by invoking tozap
#

# extract Message-Id
# $1 = article number
# $2 = Newsgroup

savesource=SAVESOURCE
num=$1
group=$2

trap "/bin/rm -f /tmp/ck$$; exit 0" 1 2 3 4 5 6 7 8 9

head -12 > /tmp/ck$$
echo "echo 'Checking: ${num} in newsgroup ${group}'" >> MYHOME/tozap

awkres=`awk  'BEGIN { sub = ""; FS = ":" }
/^Subject:/ {
	sub=$2
	for (i = 3; i <= NF; i++)
		sub = sprintf("%s %s", sub, $i);
	printf("%s", sub);
	}' < /tmp/ck$$`

subject=`echo ${awkres} | tr "'" " "`
echo "echo 'Checking:${subject}'" >> MYHOME/tozap

awk  'BEGIN { mid = "" ; FS = ":" }
/^[Mm][eE][sS][sS][aA][gG][eE]-[Ii][Dd]:/ {
		if ( mid == "" ) mid = $2;
		printf("CHKPROG \"%s\"\n", mid);
		exit
	}' < /tmp/ck$$ >> MYHOME/tozap

dir=`echo ${group} | sed -e 's,\.,\/,g'`
echo 'if [ $? != 0 ]; then' >> MYHOME/tozap
echo "echo 'Adding ${num} from ${group} to archive'" >> MYHOME/tozap
echo "${savesource} < /usr/spool/news/${dir}/${num};" >> MYHOME/tozap
echo "fi" >> MYHOME/tozap
chmod +x MYHOME/tozap
/bin/rm -f /tmp/ck$$
