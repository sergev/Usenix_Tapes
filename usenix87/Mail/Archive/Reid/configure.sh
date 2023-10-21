#! /bin/sh 
# Script to configure and install the other shell scripts for the archive
# server
#	Brian Reid, February 1987

egrep -v '%%' list | egrep -v '^$' | \
    tr -s '\011' '\011' | \
    awk -F\t '
BEGIN {printf "#! /bin/sh\nsed "}
{printf " -e '\''s+%s+%s+g'\'' \\\n",$1,$2}
END {print ""}
' > sedstr.tmp
chmod a+x sedstr.tmp
BINDIR=`echo '#BIN#' | ./sedstr.tmp`
rm -rf tmpdir.tmp
mkdir tmpdir.tmp

# now configure the executable programs
for i in *.X
do
    j=`basename $i .X`
    echo Processing $i into $j
    ./sedstr.tmp < $i > tmpdir.tmp/$j
    chmod 755 tmpdir.tmp/$j
done
mv tmpdir.tmp/install.sh .
echo Configured programs are in tmpdir.tmp. Now type '"make install"' as root
