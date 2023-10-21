#! /bin/sh
#
#	shar.sh	- shell archiver that flattens directories
#
#	usage: rshar files ... >archive
#
separator='================='
cat << 'foob'
#! /bin/sh
# This is a shell archive. It will unpack itself automatically on Unix
# systems, or you can pick it apart with a text editor. For automatic 
# unpacking, do this:
# 1. Remove everything above the #! /bin/sh line.
# 2. Save the resulting text in a file.
# 3. Unpack the file by typing "sh filename", where "filename" is the
#    name of your file. It will create these files in your current
#    directory:
foob
for f in $*
do
     echo '#	' `basename $f`
done;
echo ' '
for f in $*
do
	echo "echo extracting $f"
	echo "cat > `basename $f` << '$separator'"
	cat $f
	echo "$separator"
done
