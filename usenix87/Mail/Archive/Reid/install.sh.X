#! /bin/sh
#
# installs the archive server files
#

(mkdir #HOME#; cd #HOME#; mkdir #REQQ#; mkdir #WRKQ#; mkdir #BIN#)
cp tmpdir.tmp/* #BIN#
cp date+ #BIN#
mv #BIN#/copyright #HOME#
touch #REQQ#/in.log
touch #REQQ#/out.log
/etc/chown daemon #REQQ#/in.log #REQQ#/out.log
chmod 664 #REQQ#/in.log #REQQ#/out.log
echo 0 > #WRKQ#/seq
